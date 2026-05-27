/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(TENSORFLOW_3RDPARTY)
#include <algorithm>

#include "c_api.h"
#include "c_api_experimental.h"
#include "tensorflow_param.h"
#include "tensorflow_tools.hpp"
#include "dave_tools.h"
#include "dave_toolbox.h"
#include "party_log.h"

#define per_process_gpu_memory_fraction_default 0.1

using namespace tensorflow;
using namespace std;

class ConfigProto;

// #define ENABLE_CUDA_VISIBLE_DEVICES	// export CUDA_VISIBLE_DEVICES=0

static void
_tensorflow_free_buffer(void* data, size_t length)
{
	free(data);                                                                       
}             

static TF_Buffer *
_tensorflow_malloc_file(const char *file)
{
	FILE *f = fopen(file, "rb");

	if(f == NULL)
	{
		PARTYLOG("can't open the file:%s", file);
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	size_t fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	void *data = malloc(fsize);
	size_t rsize = fread(data, fsize, 1, f);
	if(rsize == 0)
	{
		PARTYLOG("file:%s read zero!", file);
	}
	fclose(f);

	TF_Buffer *buf = TF_NewBuffer();

	buf->data = data;
	buf->length = fsize;
	buf->data_deallocator = _tensorflow_free_buffer;

	return buf;
}

static void
_tensorflow_free_file(TF_Buffer *pBuffer)
{
	TF_DeleteBuffer(pBuffer);
}

static TF_Graph *
_tensorflow_model_open(const char *model_file)
{
	TF_Status *status;
	TF_ImportGraphDefOptions *opts; 
	TF_Buffer *graph_buffer;
	TF_Graph *pGraph = NULL;

	graph_buffer = _tensorflow_malloc_file(model_file);
	if(graph_buffer == NULL)
	{
		PARTYABNOR("read model:%s failed!", model_file);
		return NULL;
	}

	pGraph = TF_NewGraph();

	if(pGraph != NULL)
	{
		status = TF_NewStatus();
		opts = TF_NewImportGraphDefOptions();

		TF_GraphImportGraphDef(pGraph, graph_buffer, opts, status);

		if(TF_GetCode(status) != TF_OK)
		{
			PARTYLOG("unable to import graph:%s status:%s", model_file, TF_Message(status));
		}
		else
		{
			PARTYDEBUG("successfully imported graph:%s!", model_file);
		}

		TF_DeleteImportGraphDefOptions(opts);
		TF_DeleteStatus(status);
	}
	else
	{
		PARTYABNOR("TF_NewGraph failed!");
	}

	_tensorflow_free_file(graph_buffer);

	return pGraph;
}

static void
_tensorflow_model_close(TF_Graph *pGraph)
{
	if(pGraph != NULL)
	{
		TF_DeleteGraph(pGraph);
	}
}

static TF_Session *
_tensorflow_session_start(TF_Graph *pGraph)
{
	TF_Status *status;
	TF_SessionOptions *opts;
	TF_Session *session;

	status = TF_NewStatus();
	opts = TF_NewSessionOptions(per_process_gpu_memory_fraction_default);

	session = TF_NewSession(pGraph, opts, status);
	if(TF_GetCode(status) != TF_OK)
	{
		PARTYLOG("can't start session:%s", TF_Message(status));
		session = NULL;
	}

	if(status != NULL)
	{
		TF_DeleteStatus(status);
	}
	if(opts != NULL)
	{
		TF_DeleteSessionOptions(opts);
	}

	return session;
}

static void
_tensorflow_session_stop(TF_Session *pSession)
{
	TF_Status *status;

	status = TF_NewStatus();

	if(pSession != NULL)
	{
		TF_CloseSession(pSession, status);
		TF_DeleteSession(pSession, status);
	}

	if(status != NULL)
	{
		TF_DeleteStatus(status);
	}
}

static TF_Output
_tensorflow_load_ops(TF_Graph *pGraph, const char *graph_name, ub index)
{
	TF_Output ops;

	ops = TF_Output{TF_GraphOperationByName(pGraph, graph_name), (int)index};

	if(ops.oper == NULL)
	{
		PARTYLOG("can't load graph:%s", graph_name);
	}

	return ops;
}

static void
_tensorflow_setup_device(TF_Graph *pGraph, const char *device)
{
#ifdef ENABLE_CUDA_VISIBLE_DEVICES
	TF_OperationDescription *desc;

	if(device == NULL)
	{
		return;
	}

	desc = TF_NewOperation(pGraph, NULL, NULL);
	if(desc == NULL)
	{
		PARTYLOG("");
		return;
	}

	TF_SetDevice(desc, device);
#endif
}

// =====================================================================

dave_bool
tensorflow_open(
	TF_Graph **ppGraph, TF_Session **ppSession,
	TF_Output *pInputOpt, TF_Output *pOutputOpt,
	const char *input_graph, const char *output_graph,
	const char *model_file,
	const char *device)
{
	*ppGraph = NULL;
	*ppSession = NULL;

	*ppGraph = _tensorflow_model_open(model_file);
	if(*ppGraph == NULL)
	{
		PARTYLOG("open model:%s failed!", model_file);
		return dave_false;
	}

	*ppSession = _tensorflow_session_start(*ppGraph);
	if(*ppSession == NULL)
	{
		PARTYLOG("start session:%s failed!", model_file);
		tensorflow_close(*ppGraph, *ppSession); *ppGraph = NULL; *ppSession = NULL;
		return dave_false;
	}

	if((pInputOpt != NULL) && (input_graph != NULL))
	{
		*pInputOpt = _tensorflow_load_ops(*ppGraph, input_graph, 0);
		if(pInputOpt->oper == NULL)
		{
			PARTYLOG("load input ops:%s failed! model:%s", input_graph, model_file);
			tensorflow_close(*ppGraph, *ppSession); *ppGraph = NULL; *ppSession = NULL;
			return dave_false;
		}
	}

	if((pOutputOpt != NULL) && (output_graph != NULL))
	{
		*pOutputOpt = _tensorflow_load_ops(*ppGraph, output_graph, 0);
		if(pOutputOpt->oper == NULL)
		{
			PARTYLOG("load output ops:%s failed! model:%s", output_graph, model_file);
			tensorflow_close(*ppGraph, *ppSession); *ppGraph = NULL; *ppSession = NULL;
			return dave_false;
		}
	}

	_tensorflow_setup_device(*ppGraph, device);

	PARTYLOG("open graph:%s success!", model_file);

	return dave_true;
}

void
tensorflow_close(TF_Graph *pGraph, TF_Session *pSession)
{
	_tensorflow_session_stop(pSession);

	_tensorflow_model_close(pGraph);
}

TF_Tensor *
tensorflow_malloc_tensor(TF_DataType type, const int64_t *dims, int num_dims, MatrixData matrix, s8 *fun, ub line)
{
	ub data_length;
	TF_Tensor *pTensor;
	void *tensor_data;

	if(dims == NULL)
	{
		return NULL;
	}

	PARTYDEBUG("num_dims:%d len:%d", num_dims, matrix.matrix_length);

	data_length = __matrix_len__(matrix, fun, line);

	pTensor = TF_AllocateTensor(type, dims, num_dims, data_length);
	if(pTensor == NULL)
	{
		return NULL;
	}

	tensor_data = TF_TensorData(pTensor);
	if(tensor_data == NULL)
	{
       	TF_DeleteTensor(pTensor);
       	return NULL;
    }

	if(TF_TensorByteSize(pTensor) < (size_t)data_length)
	{
		PARTYLOG("data_length:%d/%d", data_length, TF_TensorByteSize(pTensor));
		data_length = TF_TensorByteSize(pTensor);
	}

    dave_memcpy(tensor_data, matrix.matrix_data, data_length);

    return pTensor;
}

void
tensorflow_free_tensor(TF_Tensor **ppTensor, ub tensor_num)
{
	ub tensor_index;

	for(tensor_index=0; tensor_index<tensor_num; tensor_index++)
	{
		if(ppTensor[tensor_index] != NULL)
		{
			TF_DeleteTensor(ppTensor[tensor_index]);
		}
	}
}

TF_Output
tensorflow_load_ops(TF_Graph *pGraph, const char *graph_name, ub index)
{
	return _tensorflow_load_ops(pGraph, graph_name, index);
}

dave_bool
__tensorflow_run__(
	MatrixData matrix,
	TF_Session *pSession,
	TF_Output *pInputOpt, ub ninputs,
	TF_Output *pOutputOpt, ub noutputs,
	TF_Tensor **output_tensors,
	ub *time,
	s8 *fun, ub line)
{
	const int64_t input_dims[] = {
		1,
		(int64_t)(matrix.dime1_depth),
		(int64_t)(matrix.dime2_depth),
		(int64_t)(matrix.dime3_depth)
	};
	TF_Tensor *input_tensors;
	TF_Status *status;
	ub run_start_time;
	dave_bool ret;

	if(time != NULL)
	{
		run_start_time = dave_os_time_us();
	}

	input_tensors = malloc_tensor(TF_FLOAT, input_dims, matrix, fun, line);
	if(input_tensors == NULL)
	{
		PARTYLOG("input_tensors is NULL!");
		return dave_false;
	}

	status = TF_NewStatus();

	TF_SessionRun(
		pSession,
		NULL,
		pInputOpt, &input_tensors, ninputs,
		pOutputOpt, output_tensors, noutputs,
		NULL, 0,
		NULL,
		status);

	if(time != NULL)
	{
		*time = dave_os_time_us() - run_start_time;
	}

	if(TF_GetCode(status) == TF_OK)
	{
		PARTYDEBUG("TF_SessionRun status:ok!");
		ret = dave_true;
	}
	else
	{
		PARTYLOG("TF_SessionRun status:%d/%s <%s:%d>",
			TF_GetCode(status), TF_Message(status),
			fun, line);
		free_tensor(output_tensors, noutputs);
		ret = dave_false;
	}

	free_tensor(&input_tensors, ninputs);

	TF_DeleteStatus(status);

	return ret;
}

#endif


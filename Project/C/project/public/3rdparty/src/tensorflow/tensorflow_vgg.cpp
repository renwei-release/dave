/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#include "tensorflow_param.h"
#if defined(TENSORFLOW_3RDPARTY) && defined(__TENSORFLOW_VGG__)
#include <vector>
#include "c_api.h"
#include "c_api_experimental.h"
#include "sample_picture_jpg_hex.hpp"
#include "tensorflow_tools.hpp"
#include "dave_define.h"
#include "dave_toolbox.h"
#include "dave_3rdparty.h"
#include "dave_tools.h"
#include "party_log.h"

using namespace std;

#define NET_DEVICE "/gpu:0"
#define VGG_CFG_OPT 1
#define INPUT_NUMBER 1
#define OUTPUT_NUMBER 1

static TF_Graph *_vgg_graph = NULL;
static TF_Session *_vgg_session = NULL;
static TF_Output _input_ops, _feature_ops;

static ub
_tensorflow_vgg_cfg(s8 *net_pb_file, s8 *input_level_name, s8 *feature_level_name, ub *row, ub *column, ub *depth)
{
	ub cfg_flag = VGG_CFG_OPT;

	if(cfg_flag == 0)
	{
		if(net_pb_file != NULL)
		{
			dave_strcpy(net_pb_file, "/dave/aix/model/vgg16_input_224x224.pb", DAVE_NORMAL_NAME_LEN);
		}
		if(input_level_name != NULL)
		{
			dave_strcpy(input_level_name, "input_1", DAVE_NORMAL_NAME_LEN);
		}
		if(feature_level_name != NULL)
		{
			dave_strcpy(feature_level_name, "global_max_pooling2d_1/Max", DAVE_NORMAL_NAME_LEN);
		}
		if(row != NULL)
		{
			*row = 224;
		}
		if(column != NULL)
		{
			*column = 224;
		}
	}
	else
	{
		if(net_pb_file != NULL)
		{
			dave_strcpy(net_pb_file, "/dave/aix/model/vgg16_input_XxX.pb", DAVE_NORMAL_NAME_LEN);
		}
		if(input_level_name != NULL)
		{
			dave_strcpy(input_level_name, "input_1", DAVE_NORMAL_NAME_LEN);
		}
		if(feature_level_name != NULL)
		{
			dave_strcpy(feature_level_name, "block4_conv3/Relu", DAVE_NORMAL_NAME_LEN);
		}
		if(row != NULL)
		{
			*row = 300;
		}
		if(column != NULL)
		{
			*column = 300;
		}
	}

	if(depth != NULL)
	{
		*depth = 3;
	}

	return cfg_flag;
}

static dave_bool
_tensorflow_vgg_open(void)
{
	s8 net_pb_file[DAVE_NORMAL_NAME_LEN];
	s8 input_level_name[DAVE_NORMAL_NAME_LEN];
	s8 feature_level_name[DAVE_NORMAL_NAME_LEN];
	dave_bool ret;

	_tensorflow_vgg_cfg(net_pb_file, input_level_name, feature_level_name, NULL, NULL, NULL);

	ret = tensorflow_open(&_vgg_graph, &_vgg_session, &_input_ops, &_feature_ops,
		(const char *)input_level_name, (const char *)feature_level_name, (const char *)net_pb_file, NET_DEVICE);

	if(ret == dave_false)
	{
		PARTYLOG("%s open failed!", net_pb_file);
	}

	return ret;
}

static void
_tensorflow_vgg_close(void)
{
	tensorflow_close(_vgg_graph, _vgg_session);

	_vgg_graph = NULL;
	_vgg_session = NULL;
}

static void
_tensorflow_vgg_feature_process(TF_Tensor *output_tensors, float *feature, ub feature_length)
{
	if(_tensorflow_vgg_cfg(NULL, NULL, NULL, NULL, NULL, NULL) == 0)
	{
		if(TF_TensorByteSize(output_tensors) > (feature_length * sizeof(float)))
		{
			PARTYLOG("tensor output short data:%d/%d", TF_TensorByteSize(output_tensors), feature_length);
		}
		else
		{
			dave_memset(feature, 0x00, feature_length * sizeof(float));
		}

		feature_length = TF_TensorByteSize(output_tensors) / sizeof(float);

		dave_memcpy(feature, TF_TensorData(output_tensors), feature_length * sizeof(float));
	}
	else
	{
		MatrixData matrix, nfeat;

		matrix.matrix_type = DaveDataType_float;
		matrix.matrix_length = TF_TensorByteSize(output_tensors) / sizeof(float);
		matrix.matrix_data = TF_TensorData(output_tensors);

		matrix.dime1_depth = 37;
		matrix.dime2_depth = 37;
		matrix.dime3_depth = 512;

		if(TF_TensorByteSize(output_tensors) != matrix_len(matrix))
		{
			PARTYABNOR("%d %d", TF_TensorByteSize(output_tensors), matrix_len(matrix));
		}

		PARTYDEBUG("matrix:%s", matrix_print(matrix));

		matrix = math_transpose(matrix, 2, 0, 1);

		nfeat = math_rmac(matrix);

		PARTYDEBUG("nfeat:%s", matrix_print(nfeat));

		if(nfeat.matrix_length != feature_length)
		{
			PARTYLOG("invalid lenght:%d/%d", matrix.matrix_length, feature_length);
			dave_memset(feature, 0x00, feature_length * sizeof(float));
		}
	
		dave_memcpy(feature, nfeat.matrix_data, matrix_len(nfeat));

		matrix_free(nfeat);

		matrix_free(matrix);
	}
}

static void
_tensorflow_vgg_normalize(float *feature, ub feature_length)
{
	MatrixData matrix;

	matrix.matrix_type = DaveDataType_float;
	matrix.matrix_length = feature_length;
	matrix.matrix_data = feature;

	matrix.dime1_depth = 1;
	matrix.dime2_depth = 1;
	matrix.dime3_depth = feature_length;

	math_normalize(matrix, 6);
}

static ub
_tensorflow_vgg_feature(MatrixData matrix, float *feature, ub feature_length)
{
	TF_Tensor *output_tensors[OUTPUT_NUMBER] = { NULL };
	ub time;
	dave_bool ret;

	if((_vgg_graph == NULL)
		|| (_vgg_session == NULL)
		|| (_input_ops.oper == NULL)
		|| (_feature_ops.oper == NULL))
	{
		PARTYLOG("can't predict!");
		return 0;
	}

	math_preprocess_input(matrix, dave_true);

	ret = tensorflow_run(
		matrix,
		_vgg_session,
		&_input_ops, INPUT_NUMBER,
		&_feature_ops, OUTPUT_NUMBER,
		output_tensors,
		&time);

	if(ret == dave_true)
	{
		if(output_tensors[0] != NULL)
		{
			PARTYDEBUG("predict success! output_tensors:%d feature_length:%d time:%ld",
				TF_TensorByteSize(output_tensors[0]), feature_length*sizeof(float), time);

			_tensorflow_vgg_feature_process(output_tensors[0], feature, feature_length);
		}
		else
		{
			PARTYABNOR("predict model failed!");
			dave_memset(feature, 0x00, feature_length * sizeof(float));
			feature_length = 0;
		}

		free_tensor(output_tensors, OUTPUT_NUMBER);
	}

	_tensorflow_vgg_normalize(feature, feature_length);

	return feature_length;
}

static void
_tensorflow_vgg_setup(Matrix *pMatrix)
{
	MatrixData *pVMatrix = &(pMatrix->net);
	ub row, column, depth;

	dave_memset(pMatrix, 0x00, sizeof(Matrix));

	pMatrix->opt = MatrixOpt_vgg;

	_tensorflow_vgg_cfg(NULL, NULL, NULL, &row, &column, &depth);

	pVMatrix->matrix_row = row;
	pVMatrix->matrix_column = column;
	pVMatrix->matrix_depth = depth;

	pVMatrix->matrix_type = DaveDataType_float;

	pVMatrix->dime1_depth = row;
	pVMatrix->dime2_depth = column;
	pVMatrix->dime3_depth = depth;
}

static ub
_tensorflow_vgg_predict(float *feature, ub feature_length, MatrixData matrix)
{
	ub row, column, depth;

	_tensorflow_vgg_cfg(NULL, NULL, NULL, &row, &column, &depth);

	matrix.dime1_depth = row;
	matrix.dime2_depth = column;
	matrix.dime3_depth = depth;

	feature_length = _tensorflow_vgg_feature(matrix, feature, feature_length);

	return feature_length;
}

static void
_tensorflow_vgg_preloading(void)
{
	Matrix matrix;	

	matrix = dave_tensorflow_vgg_feature(sample_picture_jpg_hex_ptr(), sample_picture_jpg_hex_len(), NULL);

	dave_opencv_matrix_free(&matrix);
}

// =====================================================================

extern "C" void
tensorflow_vgg_init(void)
{
	if(_tensorflow_vgg_open() == dave_true)
	{
		_tensorflow_vgg_preloading();
	}
}

extern "C" void
tensorflow_vgg_exit(void)
{
	_tensorflow_vgg_close();
}

extern "C" Matrix
__tensorflow_vgg_feature__(u8 *pic_data, ub pic_length, s8 *fun, ub line)
{
	Matrix matrix;

	_tensorflow_vgg_setup(&matrix);

	if(dave_opencv_matrix_malloc(&matrix, NULL, pic_data, pic_length) == dave_false)
	{
		PARTYLOG("Can't process the data! pic_length:%d <%s:%d>", pic_length, fun, line);
		return matrix;
	}

	matrix.net.feature_length = DAVE_VGG_FEATURE_LEN;
	matrix.net.feature_data = (float *)dave_malloc(sizeof(float) * matrix.net.feature_length);

	matrix.net.feature_length = _tensorflow_vgg_predict(matrix.net.feature_data, matrix.net.feature_length, matrix.net);

	return matrix;
}

#endif


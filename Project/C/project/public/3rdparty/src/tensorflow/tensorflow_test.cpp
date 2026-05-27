/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 *
 * https://github.com/Neargye/hello_tf_c_api
 * ================================================================================
 */

#include "3rdparty_macro.h"
#include "tensorflow_param.h"
#if defined(TENSORFLOW_3RDPARTY) && defined(__TENSORFLOW_VGG__)
#include <vector>
#include <math.h>
#include "c_api.h"
#include "c_api_experimental.h"
#include "tensorflow_tools.hpp"
#include "dave_os.h"
#include "party_log.h"

using namespace std;

#define TEST_PB_FILE "/dave/tools/python/dave-packages/image_search_engine/vgg16-pb/graph.pb"
#define INPUT_LEVEL_NAME "input_4"
#define INPUT_NUMBER 1
#define OUTPUT_LEVEL_NAME "output_node0"
#define OUTPUT_NUMBER 1

static TF_Graph *_test_graph = NULL;
static TF_Session *_test_session = NULL;
static TF_Output _input_ops, _output_ops;

static dave_bool
_tensorflow_test_open(void)
{
	if(_test_graph == NULL)
	{
		return tensorflow_open(&_test_graph, &_test_session, &_input_ops, &_output_ops,
			INPUT_LEVEL_NAME, OUTPUT_LEVEL_NAME, TEST_PB_FILE, NULL);
	}
	else
	{
		return dave_true;
	}
}

static void
_tensorflow_test_close(void)
{
	// do't close
	if(1)
	{
		tensorflow_close(_test_graph, _test_session);

		_test_graph = NULL;
		_test_session = NULL;
	}
}

static dave_bool
_tensorflow_test_predict(void)
{
	float input_vals[1*5*12] = {
		-0.4809832f, -0.3770838f, 0.1743573f, 0.7720509f, -0.4064746f, 0.0116595f, 0.0051413f, 0.9135732f, 0.7197526f, -0.0400658f, 0.1180671f, -0.6829428f,
		-0.4810135f, -0.3772099f, 0.1745346f, 0.7719303f, -0.4066443f, 0.0114614f, 0.0051195f, 0.9135003f, 0.7196983f, -0.0400035f, 0.1178188f, -0.6830465f,
		-0.4809143f, -0.3773398f, 0.1746384f, 0.7719052f, -0.4067171f, 0.0111654f, 0.0054433f, 0.9134697f, 0.7192584f, -0.0399981f, 0.1177435f, -0.6835230f,
		-0.4808300f, -0.3774327f, 0.1748246f, 0.7718700f, -0.4070232f, 0.0109549f, 0.0059128f, 0.9133330f, 0.7188759f, -0.0398740f, 0.1181437f, -0.6838635f,
		-0.4807833f, -0.3775733f, 0.1748378f, 0.7718275f, -0.4073670f, 0.0107582f, 0.0062978f, 0.9131795f, 0.7187147f, -0.0394935f, 0.1184392f, -0.6840039f,
	};
	MatrixData matrix;
	TF_Tensor *output_tensors = NULL;
	float *output_ptr;
	ub output_index;
	float output_result[4] = {-0.409784, -0.302862, 0.015259, 0.690515};
	dave_bool ret = dave_true;

	if((_test_graph == NULL)
		|| (_test_session == NULL)
		|| (_input_ops.oper == NULL)
		|| (_output_ops.oper == NULL))
	{
		PARTYLOG("can't predict!");
		return dave_false;
	}

	matrix.matrix_row = 5;
	matrix.matrix_column = 12;
	matrix.matrix_depth = 1;

	matrix.dime1_depth = 5;
	matrix.dime2_depth = 12;
	matrix.dime3_depth = 1;

	matrix.matrix_type = DaveDataType_float;
	matrix.matrix_length = sizeof(input_vals) / sizeof(float);
	matrix.matrix_data = input_vals;

	ret = tensorflow_run(
		matrix,
		_test_session,
		&_input_ops, INPUT_NUMBER,
		&_output_ops, OUTPUT_NUMBER,
		&output_tensors,
		NULL);

	if(ret == dave_true)
	{
		output_ptr = (float *)TF_TensorData(output_tensors);

		for(output_index=0; output_index<4; output_index++)
		{
			if((fabs(output_result[output_index]) - fabs(output_ptr[output_index])) > 0.000001)
			{
				PARTYLOG("index:%d result mismatch:%lf %lf",
					output_index,
					output_result[output_index], output_ptr[output_index]);
				ret = dave_false;
			}
		}

		free_tensor(&output_tensors, OUTPUT_NUMBER);
	}

	return ret;
}

// =====================================================================

extern "C" void
tensorflow_test(void)
{
	ub time_base, time_open, time_predict, time_close;
	dave_bool ret = dave_false;

	time_base = time_open = time_predict = time_close = 0;

	time_base = dave_os_time_us();

	if(_tensorflow_test_open() == dave_true)
	{
		time_open = dave_os_time_us() - time_base;

		time_base = dave_os_time_us();

		ret = _tensorflow_test_predict();

		time_predict = dave_os_time_us() - time_base;
	}
	else
	{
		time_open = dave_os_time_us() - time_base;
	}

	time_base = dave_os_time_us();

	_tensorflow_test_close();

	time_close = dave_os_time_us() - time_base;

	PARTYLOG("time open:%ld predict:%ld close:%ld ret:%s",
		time_open, time_predict, time_close,
		ret==dave_true?"success":"failed");
}

#endif


/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#include "tensorflow_param.h"
#if defined(TENSORFLOW_3RDPARTY) && defined(__TENSORFLOW_TRAFFIC__)
#include <vector>
#include "c_api.h"
#include "c_api_experimental.h"
#include "sample_picture_jpg_hex.hpp"
#include "tensorflow_tools.hpp"
#include "dave_3rdparty.h"
#include "dave_toolbox.h"
#include "dave_tools.h"
#include "party_log.h"

#define NET_DEVICE "/gpu:0"
#define NET_PB_FILE "/dave/aix/model/food_scene_classification.pb"
#define INPUT_NAME "conv1_input"
#define INPUT_NUMBER 1
#define OUTPUT_NAME "output_1"
#define OUTPUT_NUMBER 1
#define TRAFFIC_MATRIX_ROW 32
#define TRAFFIC_MATRIX_COLUMN 32
#define TRAFFIC_MATRIX_DEPTH 3

static TF_Graph *_traffic_graph = NULL;
static TF_Session *_traffic_session = NULL;
static TF_Output _input_ops, _output_ops;

static dave_bool
_tensorflow_traffic_open(void)
{
	dave_bool ret;

	ret = tensorflow_open(&_traffic_graph, &_traffic_session,
		&_input_ops, &_output_ops,
		INPUT_NAME, OUTPUT_NAME,
		NET_PB_FILE, NET_DEVICE);

	if(ret == dave_false)
	{
		PARTYLOG("%s open failed!", NET_PB_FILE);
	}

	return ret;
}

static void
_tensorflow_traffic_close(void)
{
	tensorflow_close(_traffic_graph, _traffic_session);

	_traffic_graph = NULL;
	_traffic_session = NULL;
}

static void
_tensorflow_traffic_preprocess(MatrixData matrix)
{
	float *data_float = (float *)(matrix.matrix_data);
	ub data_index;

	data_index = 0;

	while(data_index < matrix.matrix_length)
	{
		data_float[data_index ++] /= (float)255.0;
	}
}

static dave_bool
_tensorflow_traffic_predict(float *score, ub *label, Matrix *pMatrix)
{
	MatrixData *traffic_mat = &(pMatrix->net);
	MatrixData result_matrix;
	TF_Tensor *output_tensors[OUTPUT_NUMBER] = { NULL };
	dave_bool ret;

	traffic_mat->dime1_depth = TRAFFIC_MATRIX_ROW;
	traffic_mat->dime2_depth = TRAFFIC_MATRIX_COLUMN;
	traffic_mat->dime3_depth = TRAFFIC_MATRIX_DEPTH;

	_tensorflow_traffic_preprocess(*traffic_mat);

	ret = tensorflow_run(
		*traffic_mat,
		_traffic_session,
		&_input_ops, INPUT_NUMBER,
		&_output_ops, OUTPUT_NUMBER,
		output_tensors,
		NULL);

	if(ret == dave_true)
	{
		result_matrix.matrix_row = TF_TensorByteSize(output_tensors[0]) / 4;
		result_matrix.matrix_column = 1;
		result_matrix.matrix_depth = 1;

		result_matrix.matrix_type = DaveDataType_float;
		result_matrix.matrix_length = result_matrix.matrix_row;
		result_matrix.matrix_data = TF_TensorData(output_tensors[0]);

		result_matrix.dime1_depth = result_matrix.matrix_row;
		result_matrix.dime2_depth = result_matrix.matrix_column;
		result_matrix.dime3_depth = result_matrix.matrix_depth;

		PARTYDEBUG("%s", matrix_print(result_matrix));

		*score = matrix_maxvalue(result_matrix, label);

		free_tensor(output_tensors, OUTPUT_NUMBER);
	}

	return ret;
}

static  void
_tensorflow_traffic_setup(Matrix *pMatrix)
{
	MatrixData *pUMatrix = &(pMatrix->net);

	dave_memset(pMatrix, 0x00, sizeof(Matrix));

	pMatrix->opt = MatrixOpt_traffic;

	pUMatrix->matrix_row = TRAFFIC_MATRIX_ROW;
	pUMatrix->matrix_column = TRAFFIC_MATRIX_COLUMN;
	pUMatrix->matrix_depth = TRAFFIC_MATRIX_DEPTH;

	pUMatrix->matrix_type = DaveDataType_float;

	pUMatrix->dime1_depth = TRAFFIC_MATRIX_ROW;
	pUMatrix->dime2_depth = TRAFFIC_MATRIX_COLUMN;
	pUMatrix->dime3_depth = TRAFFIC_MATRIX_DEPTH;
}

static void
_tensorflow_traffic_preloading(void)
{
	float score;
	ub label;

	dave_tensorflow_traffic_predict(&score, &label, sample_picture_jpg_hex_ptr(), sample_picture_jpg_hex_len(), NULL);
}

// =====================================================================

extern "C" void
tensorflow_traffic_init(void)
{
	if(_tensorflow_traffic_open() == dave_true)
	{
		_tensorflow_traffic_preloading();
	}
}

extern "C" void
tensorflow_traffic_exit(void)
{
	_tensorflow_traffic_close();
}

extern "C" dave_bool
__tensorflow_traffic_predict__(float *score, ub *label, u8 *pic_data, ub pic_length, s8 *fun, ub line)
{
	Matrix matrix;
	dave_bool ret = dave_false;

	*score = 0;
	*label = 0;

	_tensorflow_traffic_setup(&matrix);

	if(dave_opencv_matrix_malloc(&matrix, NULL, pic_data, pic_length) == dave_false)
	{
		PARTYLOG("Can't process the data! pic_length:%d <%s:%d>", pic_length, fun, line);
	}
	else
	{
		ret = _tensorflow_traffic_predict(score, label, &matrix);
		if(ret == dave_false)
		{
			PARTYLOG("traffic predict failed! <%s:%d>", fun, line);
		}
	}

	dave_opencv_matrix_free(&matrix);

	return ret;
}

#endif


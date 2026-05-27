/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#include "tensorflow_param.h"
#if defined(TENSORFLOW_3RDPARTY) && defined(__TENSORFLOW_UNET__)
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

#define NET_DEVICE "/gpu:0"
#define NET_PB_FILE "/dave/aix/model/tf_unet.pb"
#define INPUT_NAME "input_1"
#define INPUT_NUMBER 1
#define OUTPUT_NAME "activation_6/truediv"
#define OUTPUT_NUMBER 1
#define UNET_MATRIX_ROW 192
#define UNET_MATRIX_COLUMN 192
#define UNET_MATRIX_DEPTH 3
#define UNET_CONFIDENCE 0.5

static TF_Graph *_unet_graph = NULL;
static TF_Session *_unet_session = NULL;
static TF_Output _input_ops, _output_ops;

static dave_bool
_tensorflow_unet_open(void)
{
	dave_bool ret;

	ret = tensorflow_open(&_unet_graph, &_unet_session,
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
_tensorflow_unet_close(void)
{
	tensorflow_close(_unet_graph, _unet_session);

	_unet_graph = NULL;
	_unet_session = NULL;
}

static dave_bool
_tensorflow_unet_predict(Matrix *pMatrix)
{
	MatrixData *unet_mat = &(pMatrix->net);
	TF_Tensor *output_tensors[OUTPUT_NUMBER] = { NULL };
	dave_bool ret;

	unet_mat->dime1_depth = UNET_MATRIX_ROW;
	unet_mat->dime2_depth = UNET_MATRIX_COLUMN;
	unet_mat->dime3_depth = UNET_MATRIX_DEPTH;

	math_preprocess_input(*unet_mat, dave_false);

	ret = tensorflow_run(
		*unet_mat,
		_unet_session,
		&_input_ops, INPUT_NUMBER,
		&_output_ops, OUTPUT_NUMBER,
		output_tensors,
		NULL);

	if(ret == dave_true)
	{
		MatrixData net_matrix;

		net_matrix.matrix_type = DaveDataType_float;
		net_matrix.matrix_length = TF_TensorByteSize(output_tensors[0]) / sizeof(float);
		net_matrix.matrix_data = TF_TensorData(output_tensors[0]);

		net_matrix.dime1_depth = 96;
		net_matrix.dime2_depth = 96;
		net_matrix.dime3_depth = 3;

		ret = dave_opencv_unet_post_treatment(unet_mat, net_matrix);

		free_tensor(output_tensors, OUTPUT_NUMBER);
	}

	return ret;
}

static  void
_tensorflow_unet_setup(Matrix *pMatrix)
{
	MatrixData *pUMatrix = &(pMatrix->net);

	dave_memset(pMatrix, 0x00, sizeof(Matrix));

	pMatrix->opt = MatrixOpt_unet;

	pUMatrix->matrix_row = UNET_MATRIX_ROW;
	pUMatrix->matrix_column = UNET_MATRIX_COLUMN;
	pUMatrix->matrix_depth = UNET_MATRIX_DEPTH;

	pUMatrix->matrix_type = DaveDataType_float;

	pUMatrix->dime1_depth = UNET_MATRIX_ROW;
	pUMatrix->dime2_depth = UNET_MATRIX_COLUMN;
	pUMatrix->dime3_depth = UNET_MATRIX_DEPTH;
}

static void
_tensorflow_unet_preloading(void)
{
	Matrix matrix;	

	matrix = dave_tensorflow_unet_predict(sample_picture_jpg_hex_ptr(), sample_picture_jpg_hex_len(), NULL);

	dave_opencv_matrix_free(&matrix);
}

// =====================================================================

extern "C" void
tensorflow_unet_init(void)
{
	if(_tensorflow_unet_open() == dave_true)
	{
		_tensorflow_unet_preloading();
	}
}

extern "C" void
tensorflow_unet_exit(void)
{
	_tensorflow_unet_close();
}

extern "C" Matrix
__tensorflow_unet_predict__(u8 *pic_data, ub pic_length, s8 *fun, ub line)
{
	Matrix matrix;

	_tensorflow_unet_setup(&matrix);

	if(dave_opencv_matrix_malloc(&matrix, NULL, pic_data, pic_length) == dave_false)
	{
		PARTYLOG("Can't process the data! pic_length:%d <%s:%d>", pic_length, fun, line);
		return matrix;
	}

	if(_tensorflow_unet_predict(&matrix) == dave_false)
	{
		dave_opencv_matrix_free(&matrix);		
	}

	return matrix;
}

#endif


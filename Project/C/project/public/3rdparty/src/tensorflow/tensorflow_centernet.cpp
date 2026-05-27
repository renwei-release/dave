/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#include "tensorflow_param.h"
#if defined(TENSORFLOW_3RDPARTY) && defined(__TENSORFLOW_CENTERNET__)
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
#define NET_PB_FILE "/dave/aix/model/centernet_dlav0_34_model_20191113_model_best_tf.pb"
#define INPUT_LEVEL_NAME "image"
#define INPUT_NUMBER 1
#define OUTPUT_Sigmoid_NAME "Sigmoid"
#define OUTPUT_MaxPool_NAME "transpose_184"
#define OUTPUT_split_118_NAME "split_118"
#define OUTPUT_split_123_NAME "split_123"
#define OUTPUT_NUMBER 6
#define CENTERNET_MATRIX_ROW 512
#define CENTERNET_MATRIX_COLUMN 512
#define CENTERNET_MATRIX_DEPTH 3
#define CENTERNET_CONFIDENCE 0.35

static TF_Graph *_centernet_graph = NULL;
static TF_Session *_centernet_session = NULL;
static TF_Output _input_ops, _output_ops[OUTPUT_NUMBER];

static dave_bool
_tensorflow_centernet_open(void)
{
	dave_bool ret;

	ret = tensorflow_open(&_centernet_graph, &_centernet_session, &_input_ops, NULL,
		INPUT_LEVEL_NAME, NULL, NET_PB_FILE, NET_DEVICE);

	if(ret == dave_true)
	{
		_output_ops[0] = tensorflow_load_ops(_centernet_graph, OUTPUT_Sigmoid_NAME, 0);
		_output_ops[1] = tensorflow_load_ops(_centernet_graph, OUTPUT_MaxPool_NAME, 0);
		_output_ops[2] = tensorflow_load_ops(_centernet_graph, OUTPUT_split_118_NAME, 0);
		_output_ops[3] = tensorflow_load_ops(_centernet_graph, OUTPUT_split_118_NAME, 1);
		_output_ops[4] = tensorflow_load_ops(_centernet_graph, OUTPUT_split_123_NAME, 0);
		_output_ops[5] = tensorflow_load_ops(_centernet_graph, OUTPUT_split_123_NAME, 1);
	}
	else
	{
		PARTYLOG("%s open failed!", NET_PB_FILE);
	}

	return ret;
}

static void
_tensorflow_centernet_close(void)
{
	tensorflow_close(_centernet_graph, _centernet_session);

	_centernet_graph = NULL;
	_centernet_session = NULL;
}

static float
_tensorflow_centernet_get_result(TF_Tensor *tensors, ub xs_index, ub ys_index)
{
	float *tensors_ptr = (float *)TF_TensorData(tensors);

	return tensors_ptr[ys_index * 128 + xs_index];
}

static CVRectangle
_tensorflow_centernet_rectangle(float xs, float ys, float w, float h, float reg_x, float reg_y, float base_w, float base_h)
{
	CVRectangle rectangle;

	rectangle.x1 = (xs + reg_x) - (w / 2);
	rectangle.y1 = (ys + reg_y) - (h / 2);
	rectangle.x2 = (xs + reg_x) + (w / 2);
	rectangle.y2 = (ys + reg_y) + (h / 2);

	rectangle.w = base_w;
	rectangle.h = base_h;

	return rectangle;
}

static void
_tensorflow_centernet_build_start_end_xy(ub *start_x, ub *start_y, ub *end_x, ub *end_y, float base_w, float base_h)
{
	if(base_w > base_h)
	{
		*start_x = 10;
		*end_x = 118;
		*start_y = (ub)(64.0 * (1.0 - base_h/base_w));
		*end_y = (ub)(64.0 * (1.0 + base_h/base_w));
	}
	else
	{
		*start_x = (ub)(64.0 * (1.0 - base_w/base_h));
		*end_x = (ub)(64.0 * (1.0 + base_w/base_h));
		*start_y = 10;
		*end_y = 118;
	}

	if(*start_x > 128)
	{
		*start_x = 0;
	}
	if(*end_x > 128)
	{
		*end_x = 128;
	}
	if(*start_y > 128)
	{
		*start_y = 0;
	}
	if(*end_y > 128)
	{
		*end_y = 128;
	}
}

static float
_tensorflow_centernet_predict_result(TF_Tensor **tensors, CVRectangle *pRectangle, float base_w, float base_h)
{
	float *Sigmoid_ptr = (float *)TF_TensorData(tensors[0]);
	float *MaxPool_ptr = (float *)TF_TensorData(tensors[1]);
	ub start_x, start_y, end_x, end_y;
	ub xs_index, ys_index;
	float xs, ys, w, h, reg_x, reg_y;
	CVRectangle rectangle;
	float max_score, cur_score;
	ub ptr_index;

	max_score = cur_score = 0;

	start_x = 0;
	start_y = 0;
	end_x = 128;
	end_y = 128;
	
	_tensorflow_centernet_build_start_end_xy(&start_x, &start_y, &end_x, &end_y, base_w, base_h);

	for(ys_index=start_y; ys_index<end_y; ys_index++)
	{
		for(xs_index=start_x; xs_index<end_x; xs_index++)
		{
			ptr_index = ys_index * 128 + xs_index;

			if((Sigmoid_ptr[ptr_index] == MaxPool_ptr[ptr_index])
				&& (Sigmoid_ptr[ptr_index] > 0.3))
			{
				xs = (float)xs_index;
				ys = (float)ys_index;
				w = _tensorflow_centernet_get_result(tensors[2], xs_index, ys_index);
				h = _tensorflow_centernet_get_result(tensors[3], xs_index, ys_index);
				reg_x = _tensorflow_centernet_get_result(tensors[4], xs_index, ys_index);
				reg_y = _tensorflow_centernet_get_result(tensors[5], xs_index, ys_index);

				rectangle = _tensorflow_centernet_rectangle(xs, ys, w, h, reg_x, reg_y, base_w, base_h);
				cur_score = Sigmoid_ptr[ptr_index];

				PARTYDEBUG("s:%lf/%lf w:%lf h:%lf reg:%lf/%lf\r\n[%lf %lf %lf %lf] score:%lf",
					xs, ys, rectangle.w, rectangle.h, reg_x, reg_y,
					rectangle.x1, rectangle.y1, rectangle.x2, rectangle.y2,
					cur_score);

				if(max_score < cur_score)
				{
					*pRectangle = rectangle;
					max_score = cur_score;
				}
			}
		}
	}

	return max_score;
}

static void
_tensorflow_centernet_coordinates_of_the_real_picture(CVRectangle *pRectangle, double *trans, float base_w, float base_h)
{
	double t1, t2, t3, t4, t5, t6;

	if(trans == NULL)
	{
		PARTYABNOR("empty trans!");
		return;
	}

	t1 = trans[0];
	t2 = trans[1];
	t3 = trans[2];
	t4 = trans[3];
	t5 = trans[4];
	t6 = trans[5];

	pRectangle->x1 = (float)((t1 * (double)(pRectangle->x1)) + (t2 * (double)(pRectangle->y1)) + t3);
	pRectangle->y1 = (float)((t4 * (double)(pRectangle->x1)) + (t5 * (double)(pRectangle->y1)) + t6);
	pRectangle->x2 = (float)((t1 * (double)(pRectangle->x2)) + (t2 * (double)(pRectangle->y2)) + t3);
	pRectangle->y2 = (float)((t4 * (double)(pRectangle->x2)) + (t5 * (double)(pRectangle->y2)) + t6);

	PARTYDEBUG("[%lf %lf %lf %lf]", pRectangle->x1, pRectangle->y1, pRectangle->x2, pRectangle->y2);

	if(pRectangle->x1 < 0)
	{
		pRectangle->x1 = 0;
	}
	if(pRectangle->x2 < 0)
	{
		pRectangle->x2 = 0;
	}
	if(pRectangle->y1 < 0)
	{
		pRectangle->y1 = 0;
	}
	if(pRectangle->y2 < 0)
	{
		pRectangle->y2 = 0;
	}

	pRectangle->w = base_w;
	pRectangle->h = base_h;

	if(pRectangle->x1 > (pRectangle->w - 1))
	{
		pRectangle->x1 = (pRectangle->w - 1);
	}
	if(pRectangle->x2 > (pRectangle->w - 1))
	{
		pRectangle->x2 = (pRectangle->w - 1);
	}
	if(pRectangle->y1 > (pRectangle->h - 1))
	{
		pRectangle->y1 = (pRectangle->h - 1);
	}
	if(pRectangle->y2 > (pRectangle->h - 1))
	{
		pRectangle->y2 = (pRectangle->h - 1);
	}

	if(pRectangle->x1 > pRectangle->x2)
	{
		PARTYABNOR("invalid x1:%lf x2:%lf", pRectangle->x1, pRectangle->x2);
		pRectangle->x1 = pRectangle->x2;
	}
	if(pRectangle->y1 > pRectangle->y2)
	{
		PARTYABNOR("invalid y1:%lf y2:%lf", pRectangle->y1, pRectangle->y2);
		pRectangle->y1 = pRectangle->y2;
	}
}

static dave_bool
_tensorflow_centernet_predict(MatrixData *matrix, float base_w, float base_h)
{
	TF_Tensor *output_tensors[OUTPUT_NUMBER] = { NULL, NULL, NULL, NULL, NULL, NULL };
	dave_bool ret;

	math_mean_variance(*matrix,
		0.40789654, 0.44719302, 0.47026115,
		0.28863828, 0.27408164, 0.27809835);

	ret = tensorflow_run(
		*matrix,
		_centernet_session,
		&_input_ops, INPUT_NUMBER,
		_output_ops, OUTPUT_NUMBER,
		output_tensors,
		NULL);

	if(ret == dave_true)
	{
		matrix->score = _tensorflow_centernet_predict_result(output_tensors, &(matrix->rectangle), base_w, base_h);

		_tensorflow_centernet_coordinates_of_the_real_picture(
			&(matrix->rectangle), (double *)(matrix->extra_data),
			base_w, base_h);

		free_tensor(output_tensors, OUTPUT_NUMBER);
	}

	return ret;
}

static void
_tensorflow_centernet_setup(Matrix *pMatrix)
{
	MatrixData *pCMatrix = &(pMatrix->net);

	dave_memset(pMatrix, 0x00, sizeof(Matrix));

	pMatrix->opt = MatrixOpt_centernet;

	pCMatrix->matrix_row = CENTERNET_MATRIX_ROW;
	pCMatrix->matrix_column = CENTERNET_MATRIX_COLUMN;
	pCMatrix->matrix_depth = CENTERNET_MATRIX_DEPTH;

	pCMatrix->matrix_type = DaveDataType_float;

	pCMatrix->dime1_depth = CENTERNET_MATRIX_DEPTH;
	pCMatrix->dime2_depth = CENTERNET_MATRIX_ROW;
	pCMatrix->dime3_depth = CENTERNET_MATRIX_COLUMN;
}

static void
_tensorflow_centernet_preloading(void)
{
	Matrix matrix;	

	matrix = dave_tensorflow_centernet_predict(sample_picture_jpg_hex_ptr(), sample_picture_jpg_hex_len(), NULL);

	dave_opencv_matrix_free(&matrix);
}

// =====================================================================

extern "C" void
tensorflow_centernet_init(void)
{
	if(_tensorflow_centernet_open() == dave_true)
	{
		_tensorflow_centernet_preloading();
	}
}

extern "C" void
tensorflow_centernet_exit(void)
{
	_tensorflow_centernet_close();
}

extern "C" Matrix
__tensorflow_centernet_predict__(u8 *pic_data, ub pic_length, s8 *fun, ub line)
{
	Matrix matrix;
	CVRectangle orginial_rectangle;

	_tensorflow_centernet_setup(&matrix);

	if(dave_opencv_matrix_malloc(&matrix, NULL, pic_data, pic_length) == dave_false)
	{
		PARTYLOG("Can't process the data! pic_length:%d <%s:%d>", pic_length, fun, line);
		return matrix;
	}

	matrix.net.dime1_depth = CENTERNET_MATRIX_DEPTH;
	matrix.net.dime2_depth = CENTERNET_MATRIX_ROW;
	matrix.net.dime3_depth = CENTERNET_MATRIX_COLUMN;

	if(_tensorflow_centernet_predict(&(matrix.net),
		(float)(matrix.original.matrix_column), (float)(matrix.original.matrix_row)) == dave_false)
	{
		PARTYLOG("centernet predict failed!");
		matrix.net.score = 0.0;
	}

	rectangle_reset_from_Matrix(&orginial_rectangle, matrix.original);

	if(matrix.net.score < CENTERNET_CONFIDENCE)
	{
		PARTYTRACE("the centernet score:%lf too small!", matrix.net.score);
		matrix.net.rectangle = orginial_rectangle;
	}

	if((orginial_rectangle.x1 != matrix.net.rectangle.x1)
		|| (orginial_rectangle.y1 != matrix.net.rectangle.y1)
		|| (orginial_rectangle.x2 != matrix.net.rectangle.x2)
		|| (orginial_rectangle.y2 != matrix.net.rectangle.y2))
	{
		dave_opencv_cutting(&(matrix.net), matrix.net.rectangle);
	}

	return matrix;
}

#endif


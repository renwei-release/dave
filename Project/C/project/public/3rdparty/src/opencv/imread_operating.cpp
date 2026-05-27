/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(OPENCV_3RDPARTY)
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/xfeatures2d/nonfree.hpp"
#include "opencv2/calib3d/calib3d_c.h"
#include <vector>
#include <new>
#include <iostream>
#include <cstdio>
#include <vector>
#include <algorithm>
#include "dave_os.h"
#include "dave_define.h"
#include "dave_toolbox.h"
#include "dave_tools.h"
#include "opencv_tools.hpp"
#include "party_log.h"

extern "C" void dave_opencv_matrix_free(Matrix *pMatrix);

using namespace cv;
using namespace std;
using namespace cv::xfeatures2d;

static void
_imread_matrix_malloc(MatrixData *pMatrix)
{
	pMatrix->matrix_type = DaveDataType_float;
	pMatrix->matrix_length = 0;
	pMatrix->matrix_data = NULL;
	pMatrix->raw_data_length = 0;
	pMatrix->raw_data_ptr = NULL;
	pMatrix->extra_type = DaveDataType_float;
	pMatrix->extra_length = 0;
	pMatrix->extra_data = NULL;
	pMatrix->feature_length = 0;
	pMatrix->feature_data = NULL;
} 

static void
_imread_matrix_free(MatrixData *pMatrix)
{
	matrix_free(*pMatrix);
}

static void
_imread_matrix_hwc_to_chw(MatrixData *pMatrix, Mat &mat_bgr)
{
	float *matrix_float;
	ub matrix_b_offset, matrix_g_offset, matrix_r_offset;
	ub matrix_base_index, matrix_column_index, matrix_row_index;

	matrix_float = (float *)(pMatrix->matrix_data);

	matrix_b_offset = 0;
	matrix_g_offset = matrix_b_offset + (pMatrix->matrix_column * pMatrix->matrix_row);
	matrix_r_offset = matrix_g_offset + (pMatrix->matrix_column * pMatrix->matrix_row);

	matrix_base_index = 0;

	for(matrix_column_index=0; matrix_column_index<pMatrix->matrix_column; matrix_column_index++)
	{
		for(matrix_row_index=0; matrix_row_index<pMatrix->matrix_row; matrix_row_index++)
		{
			matrix_float[matrix_b_offset] = (float)(mat_bgr.data[matrix_base_index ++]);					// B
			matrix_float[matrix_b_offset + matrix_g_offset] = (float)(mat_bgr.data[matrix_base_index ++]);	// G
			matrix_float[matrix_b_offset + matrix_r_offset] = (float)(mat_bgr.data[matrix_base_index ++]);	// R

			matrix_b_offset ++;
		}
	}
}

static dave_bool
_imread_matrix_base_opt(MatrixData *pMatrix, Mat &pic_mat, u8 *pic_data, ub pic_length, s8 *fun, ub line)
{
	if((pic_data == NULL) || (pic_length == 0))
	{
		PARTYABNOR("pic_data:%x or pic_length:%d", pic_data, pic_length);
		return dave_false;
	}

	_InputArray pic_arr(pic_data, pic_length);

	pic_mat = imdecode(pic_arr, IMREAD_COLOR);

	if(pic_mat.empty())
	{
		PARTYABNOR("pic_data:%d read error! maybe the pic is 8 Bit depth!", pic_length);
		return dave_false;
	}
	if(pic_mat.data == NULL)
	{
		PARTYABNOR("pic_data:%d read error!", pic_length);
		return dave_false;
	}

	PARTYDEBUG("rows:%d cols:%d", pic_mat.rows, pic_mat.cols);

	pMatrix->matrix_row = (ub)(pic_mat.rows);
	pMatrix->matrix_column = (ub)(pic_mat.cols);
	pMatrix->matrix_depth = 3;

	pMatrix->matrix_type = DaveDataType_char;
	pMatrix->matrix_length = pMatrix->matrix_row * pMatrix->matrix_column * pMatrix->matrix_depth;

	pMatrix->dime1_depth = pMatrix->matrix_row;
	pMatrix->dime2_depth = pMatrix->matrix_column;
	pMatrix->dime3_depth = pMatrix->matrix_depth;

	pMatrix->matrix_data = __base_malloc__(matrix_len(*pMatrix), dave_false, 0x00, fun, line);
	dave_memcpy(pMatrix->matrix_data, pic_mat.data, matrix_len(*pMatrix));

	pMatrix->raw_w = (ub)(pic_mat.cols);
	pMatrix->raw_h = (ub)(pic_mat.rows);
	pMatrix->raw_data_length = pic_length;
	if(pMatrix->raw_data_ptr != NULL)
	{
		PARTYLOG("raw_data_ptr not NULL!");
		dave_free(pMatrix->raw_data_ptr);
	}
	pMatrix->raw_data_ptr = (u8 *)__base_malloc__(pMatrix->raw_data_length, dave_false, 0x00, fun, line);
	dave_memcpy(pMatrix->raw_data_ptr, pic_data, pMatrix->raw_data_length);

	return dave_true;
}

static dave_bool
_imread_matrix_vgg_opt(MatrixData *pMatrix, Mat &pic_mat, s8 *fun, ub line)
{
	Mat vgg_mat;

	resize(pic_mat, vgg_mat, Size(pMatrix->matrix_row, pMatrix->matrix_column), 0, 0, INTER_CUBIC);

	*pMatrix = __mat_to_matrix_and_bgr_to_rgb__(vgg_mat, fun, line);

	return dave_true;
}

static dave_bool
_imread_matrix_centernet_opt(MatrixData *pMatrix, Mat &pic_mat, u8 *pic_data, ub pic_length, s8 *fun, ub line)
{
	float w, h, s;
	Point2f srcTri[3];
	Point2f dstTri[3];
	Point2f transTri[3];
	Mat AffineTransform;
	Mat TransTransform;
	Mat centernet_mat;

	w = (float)(pic_mat.cols);
	h = (float)(pic_mat.rows);
	s = w > h ? w : h;

	srcTri[0] = Point2f( w / 2.0, h / 2.0 );
	srcTri[1] = Point2f( w / 2.0, (h / 2.0) - (s / 2.0) );
	srcTri[2] = Point2f( (w / 2.0) - (s / 2.0), (h / 2.0) - (s / 2.0) );

	dstTri[0] = Point2f( 256, 256 );
	dstTri[1] = Point2f( 256, 0 );
	dstTri[2] = Point2f( 0, 0 );

	transTri[0] = Point2f( 64, 64 );
	transTri[1] = Point2f( 64, 0 );
	transTri[2] = Point2f( 0, 0 );

	AffineTransform = getAffineTransform(srcTri, dstTri);

	centernet_mat = Mat::zeros(pMatrix->matrix_row, pMatrix->matrix_column, pic_mat.type());

	warpAffine(pic_mat, centernet_mat, AffineTransform, centernet_mat.size());

	pMatrix->matrix_type = DaveDataType_float;
	pMatrix->matrix_length = pMatrix->matrix_row * pMatrix->matrix_column * pMatrix->matrix_depth;

	pMatrix->matrix_data = __base_malloc__(matrix_len(*pMatrix), dave_false, 0x00, fun, line);

	_imread_matrix_hwc_to_chw(pMatrix, centernet_mat);

	TransTransform = getAffineTransform(transTri, srcTri);

	pMatrix->raw_w = (ub)(pic_mat.cols);
	pMatrix->raw_h = (ub)(pic_mat.rows);
	pMatrix->raw_data_length = pic_length;
	if(pMatrix->raw_data_ptr != NULL)
	{
		PARTYLOG("raw_data_ptr not NULL!");
		dave_free(pMatrix->raw_data_ptr);
	}
	pMatrix->raw_data_ptr = (u8 *)__base_malloc__(pMatrix->raw_data_length, dave_false, 0x00, fun, line);
	dave_memcpy(pMatrix->raw_data_ptr, pic_data, pMatrix->raw_data_length);

	pMatrix->extra_type = DaveDataType_double;
	pMatrix->extra_length = TransTransform.cols * TransTransform.rows;
	pMatrix->extra_data = __base_malloc__(pMatrix->extra_length * sizeof(double), dave_false, 0x00, fun, line);
	dave_memcpy(pMatrix->extra_data, TransTransform.data, pMatrix->extra_length * sizeof(double));

	return dave_true;
}

static dave_bool
_imread_matrix_unet_opt(MatrixData *pMatrix, Mat &pic_mat, u8 *pic_data, ub pic_length, s8 *fun, ub line)
{
	Mat unet_mat;

	resize(pic_mat, unet_mat, Size(pMatrix->matrix_row, pMatrix->matrix_column), 0, 0, INTER_CUBIC);

	*pMatrix = __mat_to_matrix_and_bgr_to_rgb__(unet_mat, fun, line);

	pMatrix->raw_w = (ub)(pic_mat.cols);
	pMatrix->raw_h = (ub)(pic_mat.rows);
	pMatrix->raw_data_length = pic_length;
	if(pMatrix->raw_data_ptr != NULL)
	{
		PARTYLOG("raw_data_ptr not NULL!");
		dave_free(pMatrix->raw_data_ptr);
	}
	pMatrix->raw_data_ptr = (u8 *)__base_malloc__(pMatrix->raw_data_length, dave_false, 0x00, fun, line);
	dave_memcpy(pMatrix->raw_data_ptr, pic_data, pMatrix->raw_data_length);

	return dave_true;
}

static dave_bool
_imread_matrix_traffic_opt(MatrixData *pMatrix, Mat &pic_mat, s8 *fun, ub line)
{
	Mat traffic_mat;

	resize(pic_mat, traffic_mat, Size(pMatrix->matrix_row, pMatrix->matrix_column), 0, 0, INTER_CUBIC);

	*pMatrix = mat_to_matrix(traffic_mat);

	return dave_true;
}

// =====================================================================

extern "C" dave_bool
__imread_matrix_malloc__(Matrix *pMatrix, s8 *pic_path, u8 *pic_data, ub pic_length, s8 *fun, ub line)
{
	MBUF *pic_mbuf = NULL;
	Mat pic_mat;
	dave_bool ret;

	_imread_matrix_malloc(&(pMatrix->original));
	_imread_matrix_malloc(&(pMatrix->net));

	if((pic_path != NULL) && ((pic_data == NULL) || (pic_length == 0)))
	{
		pic_mbuf = dave_os_file_read_mbuf(pic_path);
		if(pic_mbuf != NULL)
		{
			pic_data = (u8 *)(pic_mbuf->payload);
			pic_length = pic_mbuf->len;
		}
		else
		{
			PARTYABNOR("invalid pic_path:%s <%s:%d>", pic_path, fun, line);
		}
	}
	else
	{
		pic_mbuf = NULL;
	}

	ret = _imread_matrix_base_opt(&(pMatrix->original), pic_mat, pic_data, pic_length, fun, line);
	if(ret == dave_true)
	{
		switch(pMatrix->opt)
		{
			case MatrixOpt_base:
					ret = dave_true;
				break;
			case MatrixOpt_vgg:
					ret = _imread_matrix_vgg_opt(&(pMatrix->net), pic_mat, fun, line);
				break;
			case MatrixOpt_centernet:
					ret = _imread_matrix_centernet_opt(&(pMatrix->net), pic_mat, pic_data, pic_length, fun, line);
				break;
			case MatrixOpt_unet:
					ret = _imread_matrix_unet_opt(&(pMatrix->net), pic_mat, pic_data, pic_length, fun, line);
				break;
			case MatrixOpt_traffic:
					ret = _imread_matrix_traffic_opt(&(pMatrix->net), pic_mat, fun, line);
				break;
			default:
					pMatrix->opt = MatrixOpt_base;
					ret = dave_true;
				break;
		}
	}

	if(pic_mbuf != NULL)
	{
		dave_mfree(pic_mbuf);
	}

	if(ret == dave_false)
	{
		PARTYLOG("read invalid image:%s/%d <%s:%d>", pic_path, pic_length, fun, line);
		dave_opencv_matrix_free(pMatrix);
		Matrix_reset(pMatrix);
	}

	return ret;
}

extern "C" void
imread_matrix_free(Matrix *pMatrix)
{
	_imread_matrix_free(&(pMatrix->original));
	_imread_matrix_free(&(pMatrix->net));

	Matrix_reset(pMatrix);
}

#endif


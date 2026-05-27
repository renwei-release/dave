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
#include "party_log.h"

using namespace cv;
using namespace std;
using namespace cv::xfeatures2d;

static ub
_mat_to_data_length(Mat &mat)
{
	int depth, one_bit_has_bytes, data_length, mat_ptr_length;

	depth = mat.depth();
	if((depth == CV_8U) || (depth == CV_8S))
	{
		one_bit_has_bytes = 1 * mat.channels();
	}
	else if((depth == CV_16U) || (depth == CV_16S))
	{
		one_bit_has_bytes = 2 * mat.channels();
	}
	else if((depth == CV_32S) || (depth == CV_32F))
	{
		one_bit_has_bytes = 4 * mat.channels();
	}
	else if(depth == CV_64F)
	{
		one_bit_has_bytes = 8 * mat.channels();
	}
	else
	{
		PARTYABNOR("invalid depth:%d", depth);
		one_bit_has_bytes = 0;
	}

	data_length = mat.rows * mat.cols * one_bit_has_bytes;

	mat_ptr_length = (mat.dataend - mat.datastart);

	if(data_length != mat_ptr_length)
	{
		PARTYABNOR("data length mismatch:%d/%d!", data_length, mat_ptr_length);
	}

	return data_length;
}

static void
_matrix_to_mat_u8(Mat &mat, MatrixData matrix)
{
	ub matrix_column_index, matrix_row_index, matrix_depth_index, matrix_base_index;
	float *float_data;
	unsigned char *u8_data;
	ub u8_data_index;

	PARTYDEBUG("row:%d/%d column:%d/%d depth:%d/%d",
		mat.rows, matrix.matrix_row,
		mat.cols, matrix.matrix_column,
		mat.depth(), matrix.matrix_depth);

	float_data = (float *)(matrix.matrix_data);
	u8_data = (unsigned char *)(mat.data);

	u8_data_index = 0;

	for(matrix_column_index=0; matrix_column_index<matrix.matrix_column; matrix_column_index++)
	{
		for(matrix_row_index=0; matrix_row_index<matrix.matrix_row; matrix_row_index++)
		{
			matrix_base_index = matrix_column_index * matrix.matrix_row * matrix.matrix_depth + matrix_row_index * matrix.matrix_depth;

			for(matrix_depth_index=0; matrix_depth_index<matrix.matrix_depth; matrix_depth_index++)
			{
				u8_data[u8_data_index ++] = (unsigned char)float_data[matrix_base_index + matrix_depth_index];
			}
		}
	}
}

static void
_matrix_to_mat_s8(Mat &mat, MatrixData matrix)
{
	ub matrix_column_index, matrix_row_index, matrix_depth_index, matrix_base_index;
	float *float_data;
	char *s8_data;
	ub s8_data_index;

	PARTYDEBUG("row:%d/%d column:%d/%d depth:%d/%d",
		mat.rows, matrix.matrix_row,
		mat.cols, matrix.matrix_column,
		mat.depth(), matrix.matrix_depth);

	float_data = (float *)(matrix.matrix_data);
	s8_data = (char *)(mat.data);

	s8_data_index = 0;

	for(matrix_column_index=0; matrix_column_index<matrix.matrix_column; matrix_column_index++)
	{
		for(matrix_row_index=0; matrix_row_index<matrix.matrix_row; matrix_row_index++)
		{
			matrix_base_index = matrix_column_index * matrix.matrix_row * matrix.matrix_depth + matrix_row_index * matrix.matrix_depth;

			for(matrix_depth_index=0; matrix_depth_index<matrix.matrix_depth; matrix_depth_index++)
			{
				s8_data[s8_data_index ++] = (char)float_data[matrix_base_index + matrix_depth_index];
			}
		}
	}
}

static void
_matrix_to_rows_cols(int *rows, int *cols, MatrixData matrix)
{
	if(matrix.dime1_depth == 1)
	{
		*rows = matrix.dime2_depth;
		*cols = matrix.dime3_depth;
	}
	else
	{
		*rows = matrix.dime1_depth;
		*cols = matrix.dime2_depth;
	}
}

// =====================================================================

ub
mat_to_data_length(Mat &mat)
{
	return _mat_to_data_length(mat);
}

void
mat_print(const char *msg, Mat &pMat)
{
	int cols_index, rows_index, ptr_index;
	unsigned char *char_ptr = (unsigned char *)(pMat.data);
	float *float_ptr = (float *)(pMat.data);
	double *double_ptr = (double *)(pMat.data);

	DAVELOG("%s %d*%d\r\n", msg, pMat.rows, pMat.cols);
	DAVELOG("[\r\n");

	for(rows_index=0; rows_index<pMat.rows; rows_index++)
	{
		DAVELOG("[");

		for(cols_index=0; cols_index<pMat.cols; cols_index++)
		{
			ptr_index = rows_index * pMat.cols + cols_index;

			switch(pMat.type())
			{
				case CV_8U:
				case CV_8S:
						DAVELOG("%d", char_ptr[ptr_index]);
					break;
				case CV_32S:
				case CV_32F:
						DAVELOG("%lf", float_ptr[ptr_index]);
					break;
				case CV_64F:
						DAVELOG("%lf", double_ptr[ptr_index]);
					break;
				default:
						DAVELOG("<%d>", pMat.type());
					break;
			}
			if((cols_index + 1) < pMat.cols)
			{
				DAVELOG(" ");
			}
		}
		DAVELOG("]\r\n");			
	}

	DAVELOG("]\r\n");
}

void
mat_save(Mat &mat, const char *fmt, ...)
{
	va_list args;
	char file_name[1024];

	va_start(args, fmt);
	vsnprintf(file_name, sizeof(file_name), fmt, args);
	va_end(args);

	imwrite((const String)file_name, mat);
}

MatrixData
__mat_to_matrix_and_bgr_to_rgb__(Mat &mat_bgr, s8 *fun, ub line)
{
	MatrixData matrix;
	float *matrix_float;
	ub matrix_column_index, matrix_row_index, matrix_base_index;

	matrix = __matrix_malloc__(DaveDataType_float, mat_bgr.rows, mat_bgr.cols, mat_bgr.channels(), fun, line);

	matrix_float = (float *)(matrix.matrix_data);

	for(matrix_column_index=0; matrix_column_index<matrix.matrix_column; matrix_column_index++)
	{
		matrix_base_index = matrix_column_index * matrix.matrix_row * matrix.matrix_depth;

		for(matrix_row_index=0; matrix_row_index<matrix.matrix_row; matrix_row_index++)
		{
			matrix_float[matrix_base_index] = (float)(mat_bgr.data[matrix_base_index + 2]);		// R
			matrix_float[matrix_base_index + 1] = (float)(mat_bgr.data[matrix_base_index + 1]);	// G
			matrix_float[matrix_base_index + 2] = (float)(mat_bgr.data[matrix_base_index]);		// B

			matrix_base_index += matrix.matrix_depth;
		}
	}

	return matrix;
}

MatrixData
mat_to_matrix(Mat &mat)
{
	MatrixData matrix;
	float *matrix_float;
	ub matrix_column_index, matrix_row_index, matrix_base_index;

	matrix = matrix_malloc(DaveDataType_float, mat.rows, mat.cols, mat.channels());

	matrix_float = (float *)(matrix.matrix_data);

	for(matrix_column_index=0; matrix_column_index<matrix.matrix_column; matrix_column_index++)
	{
		matrix_base_index = matrix_column_index * matrix.matrix_row * matrix.matrix_depth;

		for(matrix_row_index=0; matrix_row_index<matrix.matrix_row; matrix_row_index++)
		{
			matrix_float[matrix_base_index] = (float)(mat.data[matrix_base_index]);
			matrix_float[matrix_base_index + 1] = (float)(mat.data[matrix_base_index + 1]);
			matrix_float[matrix_base_index + 2] = (float)(mat.data[matrix_base_index + 2]);

			matrix_base_index += matrix.matrix_depth;
		}
	}

	return matrix;
}

void
matrix_to_mat(Mat &mat, int type, MatrixData matrix)
{
	int row, column;

	_matrix_to_rows_cols(&row, &column, matrix);

	mat.create(row, column, type);

	switch(type)
	{
		case CV_8U:
				_matrix_to_mat_u8(mat, matrix);
			break;
		case CV_8S:
				_matrix_to_mat_s8(mat, matrix);
			break;
		default:
				PARTYLOG("invalid type:%d", type);
			break;
	}
}

void
__mat_encode_to_matrix_raw_data__(MatrixData *pMatrix, Mat &mat, s8 *fun, ub line)
{
    std::vector<uchar> data_encode;

    imencode(".jpg", mat, data_encode);

	if(pMatrix->raw_data_length < data_encode.size())
	{
		PARTYDEBUG("invalid raw_data_length:%d/%d",
			pMatrix->raw_data_length,
			data_encode.size());

		if(pMatrix->raw_data_ptr != NULL)
		{
			dave_free(pMatrix->raw_data_ptr);
		}
		pMatrix->raw_data_length = (ub)(data_encode.size());
		pMatrix->raw_data_ptr = (u8 *)__base_malloc__(pMatrix->raw_data_length, dave_false, 0x00, fun, line);
	}
	else
	{
		pMatrix->raw_data_length = data_encode.size();
	}

	pMatrix->matrix_row = mat.rows;
	pMatrix->matrix_column = mat.cols;

	dave_memcpy(pMatrix->raw_data_ptr, &data_encode[0], pMatrix->raw_data_length);
}

#endif


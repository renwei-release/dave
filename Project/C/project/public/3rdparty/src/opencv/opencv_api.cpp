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
#include "opencv2/core/core_c.h"
#include <vector>
#include <new>
#include <iostream>
#include <cstdio>
#include <vector>
#include <algorithm>
#include "dave_define.h"
#include "dave_toolbox.h"
#include "dave_os.h"
#include "dave_tools.h"
#include "party_log.h"

// =====================================================================

extern "C" void *
dave_opencv_cvCreateMat(ub rows, ub cols)
{
	return cvCreateMat((int)rows, (int)cols, CV_32FC1);
}

extern "C" void
dave_opencv_cvReleaseMat(void *ptr)
{
	CvMat *pCvMat = (CvMat *)ptr;

	if(pCvMat != NULL)
	{
		cvReleaseMat(&pCvMat);
	}
}

extern "C" dave_bool
dave_opencv_cvSetData(void *ptr, void *data)
{
	CvMat *pCvMat = (CvMat *)ptr;

	cvSetData(pCvMat, data, pCvMat->step);

	return dave_true;
}

extern "C" void *
dave_opencv_cvGetData(void *ptr, ub *rows, ub *cols)
{
	CvMat *pCvMat = (CvMat *)ptr;

	if(pCvMat == NULL)
	{
		if(rows != NULL)
		{
			*rows = 0;
		}
		if(cols != NULL)
		{
			*cols = 0;
		}
		return NULL;
	}

	if(rows != NULL)
	{
		*rows = pCvMat->rows;
	}
	if(cols != NULL)
	{
		*cols = pCvMat->cols;
	}

	return (void *)(pCvMat->data.fl);
}

extern "C" dave_bool
__dave_opencv_MatrixFillCv__(void *ptr, ub rows_index, MatrixData matrix, s8 *file, ub line)
{
	CvMat *pCvMat = (CvMat *)ptr;

	if((int)rows_index >= pCvMat->rows)
	{
		PARTYLTRACE(60,1,"rows:%d/%d mismatch! column:%d/%d/%d <%s:%d>",
			pCvMat->rows, rows_index,
			pCvMat->cols, matrix.matrix_column, matrix.matrix_length,
			file, line);
		return dave_false;
	}

	if((pCvMat->cols != (int)(matrix.matrix_column)) || (pCvMat->cols != (int)(matrix.matrix_length)))
	{
		PARTYLTRACE(60,1,"column:%d/%d/%d mismatch! rows:%d/%d <%s:%d>",
			pCvMat->cols, matrix.matrix_column, matrix.matrix_length,
			rows_index, pCvMat->rows,
			file, line);
		return dave_false;
	}

	dave_memcpy(&(pCvMat->data.fl[rows_index * pCvMat->cols]), matrix.matrix_data, matrix_len(matrix));

	return dave_true;
}

extern "C" dave_bool
__dave_opencv_CvFillMatrix__(MatrixData matrix, void *ptr, ub rows_index, s8 *file, ub line)
{
	CvMat *pCvMat = (CvMat *)ptr;

	if((int)rows_index >= pCvMat->rows)
	{
		PARTYLTRACE(60,1,"rows:%d/%d mismatch! column:%d <%s:%d>",
			pCvMat->rows, rows_index, pCvMat->cols,
			file, line);
		return dave_false;
	}

	if(pCvMat->cols != (int)(matrix.matrix_length))
	{
		PARTYLTRACE(60,1,"column:%d/%d mismatch! rows:%d <%s:%d>",
			pCvMat->cols, matrix.matrix_length, pCvMat->rows,
			file, line);
		return dave_false;
	}

	dave_memcpy(matrix.matrix_data, &(pCvMat->data.fl[rows_index * pCvMat->cols]), matrix_len(matrix));

	return dave_true;
}

#endif


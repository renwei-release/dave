/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_OPENCV_H__
#define __DAVE_OPENCV_H__
#include "cv_param.h"

void dave_opencv_init(void);

void dave_opencv_exit(void);

dave_bool dave_opencv_features(u8 *pic_data, ub pic_length, ub nfeatures, CVKeyPoint *pPoint, OpenCVMat *pMat);

ub dave_opencv_matcher(
	ub opt_number,
	ub *inliners_num, float *inliners_score,
	ub *keypoints_num, float *keypoints_score,
	u8 *pic_data, ub pic_length,
	CVKeyPoint *pPointDataset, OpenCVMat *pMatDataset);

dave_bool dave_opencv_matcher_on_image_path(
	ub *inliners_num, float *inliners_score,
	ub *keypoint_num, float *keypoint_score,
	s8 *image_path_a, s8 *image_path_b, ub nfeatures);

void dave_opencv_surf(s8 *image_path, ub *time);

dave_bool __dave_opencv_matrix_malloc__(Matrix *pMatrix, s8 *pic_path, u8 *pic_data, ub pic_length, s8 *fun, ub line);
#define dave_opencv_matrix_malloc(pMatrix, pic_path, pic_data, pic_length) __dave_opencv_matrix_malloc__(pMatrix, pic_path, pic_data, pic_length, (s8 *)__func__, (ub)__LINE__)

void dave_opencv_matrix_free(Matrix *pMatrix);

dave_bool dave_opencv_cutting(MatrixData *pMatrix, Rectangle rectangle);

dave_bool dave_opencv_unet_post_treatment(MatrixData *target, MatrixData net_data);

void * dave_opencv_cvCreateMat(ub rows, ub cols);

void dave_opencv_cvReleaseMat(void *ptr);

dave_bool dave_opencv_cvSetData(void *ptr, void *data);

void * dave_opencv_cvGetData(void *ptr, ub *rows, ub *cols);

dave_bool __dave_opencv_MatrixFillCv__(void *ptr, ub rows_index, MatrixData matrix, s8 *file, ub line);
#define dave_opencv_MatrixFillCv(ptr, rows_index, matrix) __dave_opencv_MatrixFillCv__(ptr, rows_index, matrix, (s8 *)__func__, (ub)__LINE__)

dave_bool __dave_opencv_CvFillMatrix__(MatrixData matrix, void *ptr, ub rows_index, s8 *file, ub line);
#define dave_opencv_CvFillMatrix(matrix, ptr, rows_index) __dave_opencv_CvFillMatrix__(matrix, ptr, rows_index, (s8 *)__func__, (ub)__LINE__)

dave_bool dave_opencv_cvCalcPCA(void *pMean, void *pEigVals, void *pEigVecs, void *pData);

dave_bool dave_opencv_cvProjectPCA(void *pPCA, void *pData, void *pMean, void *pEigVecs);

dave_bool dave_opencv_sift_valid_size(ub size);

ub dave_opencv_mat_to_nfeatures(OpenCVMat *pMat);

#endif


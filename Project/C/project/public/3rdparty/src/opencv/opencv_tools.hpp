/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __OPENCV_TOOLS_HPP__
#define __OPENCV_TOOLS_HPP__
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/xfeatures2d/nonfree.hpp"
#include "opencv2/calib3d/calib3d_c.h"
#include <vector>

using namespace cv;
using namespace std;
using namespace cv::xfeatures2d;

ub mat_to_data_length(Mat &mat);

void mat_print(const char *msg, Mat &pMat);

void mat_save(Mat &mat, const char *fmt, ...);

dave_bool picture_cutting(Matrix *pMatrix, CVRectangle *pRectangle);

MatrixData __mat_to_matrix_and_bgr_to_rgb__(Mat &mat_bgr, s8 *fun, ub line);
#define mat_to_matrix_and_bgr_to_rgb(mat_bgr) __mat_to_matrix_and_bgr_to_rgb__(mat_bgr, (s8 *)__func__, (ub)__LINE__)

MatrixData mat_to_matrix(Mat &mat);

void matrix_to_mat(Mat &mat, int type, MatrixData matrix);

void __mat_encode_to_matrix_raw_data__(MatrixData *pMatrix, Mat &mat, s8 *fun, ub line);
#define mat_encode_to_matrix_raw_data(pMatrix, mat) __mat_encode_to_matrix_raw_data__(pMatrix, mat, (s8 *)__func__, (ub)__LINE__)

#endif


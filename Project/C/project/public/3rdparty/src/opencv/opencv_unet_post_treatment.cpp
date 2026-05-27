/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(OPENCV_3RDPARTY)
#include "dave_define.h"
#include "dave_toolbox.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/xfeatures2d/nonfree.hpp"
#include "opencv2/calib3d/calib3d_c.h"
#include "NumCpp.hpp"
#include <vector>
#include <new>
#include <iostream>
#include <cstdio>
#include <vector>
#include <algorithm>
#include "dave_os.h"
#include "dave_tools.h"
#include "opencv_tools.hpp"
#include "party_log.h"

#define epsilon_ratio 0.05

using namespace cv;
using namespace std;
using namespace cv::xfeatures2d;
using namespace nc;

static ub
_opencv_warpPerspective_find_big_contour(vector<vector<Point>> &contours)
{
	double maxArea;
	ub max_index;
	ub index;

	maxArea = 0;
	max_index = 0;

	for(index=0; index<contours.size(); index++)
	{
		double area = cv::contourArea(contours[index]);

		if (area > maxArea)
		{
			maxArea = area;
			max_index = index;
		}
	}

	return max_index;
}

static dave_bool
_unet_post_treatment_find_convex_hull(vector<Point> &approx, nc::NdArray<uint8> &nc_array)
{
	/*
	 * convert to OpenCV Mat
	 * https://github.com/dpilger26/NumCpp/blob/master/examples/InterfaceWithOpenCV/InterfaceWithOpenCV.cpp
	 */
	auto pr = cv::Mat(nc_array.numRows(), nc_array.numCols(), CV_8U, nc_array.data());
	cv::Mat thresh;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	ub big_contour_index;
	double epsilon;
	vector<Point> hull;

	PARTYDEBUG("cols:%d rows:%d numRows:%d numCols:%d", pr.cols, pr.rows, nc_array.numRows(), nc_array.numCols());

	cv::threshold(pr, thresh, 1, 255, cv::THRESH_BINARY);

	cv::findContours(thresh, contours, hierarchy, 2, 1);

	big_contour_index = _opencv_warpPerspective_find_big_contour(contours);

	PARTYDEBUG("contours size:%d big_contour_index:%d", contours.size(), big_contour_index);

	epsilon = epsilon_ratio * cv::arcLength(contours[big_contour_index], true);

	cv::convexHull(contours[big_contour_index], hull);

	cv::approxPolyDP(hull, approx, epsilon, true);

	PARTYDEBUG("size:%d", approx.size());

	/*
	 * 只有带四个顶点的凸包形状才是我们需要的。
	 */
	if(approx.size() != 4)
	{
		return dave_false;
	}

	return dave_true;	
}

static void
_opencv_warpPerspective_order_points(nc::NdArray<float> &rect, nc::NdArray<float> &pts)
{
	nc::NdArray<float> s;
	nc::NdArray<float> diff;
	int32 min_index, max_index;

	/*
	 * 注意：与Python版本对比，NumCpp调用AxIS::COL才代表NumPy里面的1操作。
	 */
	s = nc::sum(pts, Axis::COL);
	min_index = nc::argmin(s)[0];
	max_index = nc::argmax(s)[0];
	rect.put(0, 0, pts.at(min_index, 0));
	rect.put(0, 1, pts.at(min_index, 1));
	rect.put(2, 0, pts.at(max_index, 0));
	rect.put(2, 1, pts.at(max_index, 1));

	/*
	 * 注意：与Python版本对比，NumCpp调用AxIS::COL才代表NumPy里面的1操作。
	 */
	diff = nc::diff(pts, Axis::COL);
	min_index = nc::argmin(diff)[0];
	max_index = nc::argmax(diff)[0];
	rect.put(1, 0, pts.at(min_index, 0));
	rect.put(1, 1, pts.at(min_index, 1));
	rect.put(3, 0, pts.at(max_index, 0));
	rect.put(3, 1, pts.at(max_index, 1));
}

static dave_bool
_unet_post_treatment_four_point_transform(MatrixData *target, Mat &raw_image, nc::NdArray<float> &pts)
{
	nc::NdArray<float> rect(4, 2);
	nc::NdArray<float> tl(1, 2), tr(1, 2), br(1, 2), bl(1, 2);
	
	rect.zeros();
	tl.zeros();
	tr.zeros();
	br.zeros();
	bl.zeros();

	_opencv_warpPerspective_order_points(rect, pts);

	tl.put(0, rect.at(0, 0));
	tl.put(1, rect.at(0, 1));
	tr.put(0, rect.at(1, 0));
	tr.put(1, rect.at(1, 1));
	br.put(0, rect.at(2, 0));
	br.put(1, rect.at(2, 1));
	bl.put(0, rect.at(3, 0));
	bl.put(1, rect.at(3, 1));

	double widthA = nc::sqrt(pow(tr[0] - tl[0], 2) + pow(tr[1] - tl[1], 2));
	double widthB = nc::sqrt(pow(br[0] - bl[0], 2) + pow(br[1] - bl[1], 2));
	float maxWidth = max(int(widthA), int(widthB));

	double heightA = nc::sqrt(pow(tr[0] - br[0], 2) + pow(tr[1] - br[1], 2));
	double heightB = nc::sqrt(pow(tl[0] - bl[0], 2) + pow(tl[1] - bl[1], 2));
	float maxHeight = max(int(heightA), int(heightB));

	PARTYDEBUG("maxWidth:%lf maxHeight:%lf", maxWidth, maxHeight);

	nc::NdArray<float> dst({{0, 0}, {maxWidth - 1, 0}, {maxWidth - 1, maxHeight - 1}, {0, maxHeight - 1}});

	/*
	 * convert to OpenCV Mat
	 * https://github.com/dpilger26/NumCpp/blob/master/examples/InterfaceWithOpenCV/InterfaceWithOpenCV.cpp
	 */
    auto cvArray_rect = cv::Mat(rect.numRows(), rect.numCols(), CV_32FC1, rect.data());
	auto cvArray_dst = cv::Mat(dst.numRows(), dst.numCols(), CV_32FC1, dst.data());
	Mat output_image = Mat::zeros(raw_image.rows, raw_image.cols, raw_image.type());
	Mat M = getPerspectiveTransform(cvArray_rect, cvArray_dst);
	Size dsize = {(int)maxWidth, (int)maxHeight};

	warpPerspective(raw_image, output_image, M, dsize);

	mat_encode_to_matrix_raw_data(target, output_image);

	return dave_true;
}

static dave_bool
_unet_post_treatment_get_wraped_from_pr(MatrixData *target, Mat &raw_image, nc::NdArray<uint8> &pr)
{
	vector<Point> approx;
	double raw_h, raw_w, pic_rows, pic_cols;
	int32 approx_index;
	float h, w;

	if(_unet_post_treatment_find_convex_hull(approx, pr) == dave_false)
	{
		return dave_false;
	}

	raw_h = (double)(raw_image.rows);
	raw_w = (double)(raw_image.cols);
	pic_rows = (double)(pr.numRows());
	pic_cols = (double)(pr.numCols());

	nc::NdArray<float> pts(approx.size(), 2);

	pts.zeros();

	PARTYDEBUG("raw_h:%lf raw_w:%lf pic_rows:%lf pic_cols:%lf pts.cols:%d pts.rows:%d",
		raw_h, raw_w, pic_rows, pic_cols, pts.numCols(), pts.numRows());

	for(approx_index=0; approx_index<(int32)(approx.size()); approx_index++)
	{
		PARTYDEBUG("h:%d w:%d", approx[approx_index].y, approx[approx_index].x);

		w = round(((double)(approx[approx_index].x) / pic_cols) * raw_w);
		h = round(((double)(approx[approx_index].y) / pic_rows) * raw_h);

		pts.put(approx_index, 0, w);
		pts.put(approx_index, 1, h);
	}

	return _unet_post_treatment_four_point_transform(target, raw_image, pts);
}

// =====================================================================

extern "C" dave_bool
dave_opencv_unet_post_treatment(MatrixData *target, MatrixData net_data)
{
	_InputArray pic_arr(target->raw_data_ptr, target->raw_data_length);
	Mat raw_image = imdecode(pic_arr, IMREAD_COLOR);

	if(raw_image.empty())
	{
		PARTYABNOR("pic_data:%d read error! maybe the pic is 8 Bit depth!", target->raw_data_length);
		return dave_false;
	}
	if(raw_image.data == NULL)
	{
		PARTYABNOR("pic_data:%d read error!", target->raw_data_length);
		return dave_false;
	}

	/*
	 * 需要有一个能直接定义3维数组的NumCpp，目前还未发现有这个能力。
	 */
	net_data = math_argmax(net_data, 2);

	PARTYDEBUG("%d %d", sizeof(uint32), sizeof(int));

	matrix_astype(net_data, DaveDataType_char);

	nc::NdArray<uint8> pr((uint8 *)net_data.matrix_data, net_data.dime2_depth, net_data.dime3_depth);	

	_unet_post_treatment_get_wraped_from_pr(target, raw_image, pr);

	matrix_free(net_data);

	return dave_true;
}

#endif


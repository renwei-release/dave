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

using namespace cv;
using namespace std;
using namespace cv::xfeatures2d;

// =====================================================================

extern "C" dave_bool
dave_opencv_cutting(MatrixData *pMatrix, CVRectangle rectangle)
{
	Mat pic_mat;
	int x1, x2, y1, y2;

	_InputArray pic_arr(pMatrix->raw_data_ptr, pMatrix->raw_data_length);
	pic_mat = imdecode(pic_arr, IMREAD_COLOR);

	if(pic_mat.empty())
	{
		PARTYABNOR("pic_data:%d read error!", pMatrix->raw_data_length);
		return dave_false;
	}
	if(pic_mat.data == NULL)
	{
		PARTYABNOR("pic_data:%d read error!", pMatrix->raw_data_length);
		return dave_false;
	}

	x1 = (int)(rectangle.x1);
	x2 = (int)(rectangle.x2);
	y1 = (int)(rectangle.y1);
	y2 = (int)(rectangle.y2);

	if(x2 > (pic_mat.cols - 1)) x2 = pic_mat.cols - 1;
	if(y2 > (pic_mat.rows - 1)) y2 = pic_mat.rows - 1;

	if(x1 < 0) x1 = 0;
	if(x2 < 0) x2 = 0;
	if(y1 < 0) y1 = 0;
	if(y2 < 0) y2 = 0;

	if(x1 > x2) x1 = x2;
	if(y1 > y2) y1 = y2;

	PARTYDEBUG("cols[%lf %lf %d] rows[%lf %lf %d] [%d %d %d %d]",
		rectangle.x1, rectangle.x2, pic_mat.cols,
		rectangle.y1, rectangle.y2, pic_mat.rows,
		x1, x2, y1, y2);

	Mat cutting_mat = pic_mat(Range(y1, y2), Range(x1, x2));

	mat_encode_to_matrix_raw_data(pMatrix, cutting_mat);

	return dave_true;
}

#endif


/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 *
 * https://github.com/ducha-aiki/pymagsac/tree/master/src/pymagsac/src
 * ================================================================================
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

static double ransacReprojThreshold = 5;

// =====================================================================

void
ransac_findHomography(ub *inliners_size, ub *inliners_match_size, vector<DMatch> &matches, std::vector<cv::KeyPoint> &keypoints_a, std::vector<cv::KeyPoint> &keypoints_b)
{
	ub inliners_index;
	vector<Point2f> srcPoints(matches.size());
	vector<Point2f> dstPoints(matches.size());
	vector<uchar> inliersMask(matches.size());

	for(size_t i=0; i<matches.size(); i++)
	{
		srcPoints[i] = keypoints_a[matches[i].queryIdx].pt;
		dstPoints[i] = keypoints_b[matches[i].trainIdx].pt;
	}

	findHomography(srcPoints, dstPoints, CV_RANSAC, ransacReprojThreshold, inliersMask);
	
	*inliners_size = inliersMask.size();
	*inliners_match_size = 0;

	for(inliners_index=0; inliners_index<(*inliners_size); inliners_index++)
	{
		if(inliersMask[inliners_index] == 1)
		{
			(*inliners_match_size) ++;
		}
	}
}

#endif


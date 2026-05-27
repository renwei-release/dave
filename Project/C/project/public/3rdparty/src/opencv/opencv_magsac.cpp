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

int findHomography_(std::vector<double>& srcPts,
                    std::vector<double>& dstPts,
                    std::vector<bool>& inliers,
                    std::vector<double>& H,
                    double sigma_th = 3.0,
                    double conf = 0.99,
                    int max_iters = 10000,
                    int partition_num = 5);

// =====================================================================

void
magsac_findHomography(ub *inliners_size, ub *inliners_match_size, vector<DMatch> &matches, std::vector<cv::KeyPoint> &keypoints_a, std::vector<cv::KeyPoint> &keypoints_b)
{
	ub index, matches_index, base_size = matches.size()*2;
	std::vector<double> srcPoints(base_size);
	std::vector<double> dstPoints(base_size);
	std::vector<bool> inliersMask(base_size);
	std::vector<double> H(matches.size());

	index = matches_index = 0;

	while((index + 2) <= base_size)
	{
		srcPoints[index] = (double)(keypoints_a[matches[matches_index].queryIdx].pt.x);
		srcPoints[index + 1] = (double)(keypoints_a[matches[matches_index].queryIdx].pt.y);
		dstPoints[index] = (double)(keypoints_b[matches[matches_index].trainIdx].pt.x);
		dstPoints[index + 1] = (double)(keypoints_b[matches[matches_index].trainIdx].pt.y);

		index += 2;
		matches_index ++;
	}

	findHomography_(srcPoints, dstPoints, inliersMask, H);

	*inliners_size = inliersMask.size();
	*inliners_match_size = 0;

	for(index=0; index<(*inliners_size); index++)
	{
		if(inliersMask[index] == true)
		{
			(*inliners_match_size) ++;
		}
	}
}

#endif


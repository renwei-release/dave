/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(OPENCV_3RDPARTY)
#include <opencv2/imgcodecs.hpp>
#include <opencv2/xfeatures2d/cuda.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/xfeatures2d/nonfree.hpp"
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

static dave_bool
_features_surf_gpu(s8 *image_path, ub *time)
{
	ub start_time;
	cv::cuda::GpuMat gmat;
	cv::cuda::GpuMat keypt;
	cv::cuda::GpuMat desc;

	gmat.upload(cv::imread((char *)image_path, cv::IMREAD_GRAYSCALE));

	cv::cuda::SURF_CUDA surf(500);

	if(time != NULL)
	{
		start_time = dave_os_time_us();
	}

	surf(gmat, cv::cuda::GpuMat(), keypt, desc);

	if(time != NULL)
	{
		*time = dave_os_time_us() - start_time;
	}

	return dave_true;
}

static dave_bool
_features_surf_cpu(s8 *image_path, ub *time)
{
	ub start_time;
	cv::Ptr<Feature2D> surf = cv::xfeatures2d::SURF::create();
	std::vector<cv::KeyPoint> kerpoints;

	Mat input_image = imread((char *)image_path);

	if(time != NULL)
	{
		start_time = dave_os_time_us();
	}

	surf->detect(input_image, kerpoints);

	if(time != NULL)
	{
		*time = dave_os_time_us() - start_time;
	}

	return dave_true;
}

// =====================================================================

extern "C" dave_bool
features_surf(s8 *image_path, ub *time, dave_bool gpu)
{
	dave_bool ret;

	if(gpu == dave_true)
	{
		ret = _features_surf_gpu(image_path, time);
	}
	else
	{
		ret = _features_surf_cpu(image_path, time);
	}

	return ret;
}

#endif


/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(OPENCV_3RDPARTY)
#include "dave_define.h"
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
#include "dave_tools.h"
#include "features_sift.hpp"
#include "features_surf.hpp"
#include "imread_operating.hpp"
#include "opencv_tools.hpp"
#include "party_log.h"

// =====================================================================

extern "C" void
dave_opencv_init(void)
{
	PARTYLOG("opencv version:%s", CV_VERSION);
}

extern "C" void
dave_opencv_exit(void)
{

}

extern "C" dave_bool
dave_opencv_features(u8 *pic_data, ub pic_length, ub nfeatures, CVKeyPoint *pPoint, OpenCVMat *pMat)
{
	return features_sift(pic_data, pic_length, nfeatures, pPoint, pMat);
}

extern "C" ub
dave_opencv_matcher(
	ub opt_number,
	ub *inliners_num, float *inliners_score,
	ub *keypoints_num, float *keypoints_score,
	u8 *pic_data, ub pic_length,
	CVKeyPoint *pPointDataset, OpenCVMat *pMatDataset)
{
	return matcher_sift(
		opt_number,
		inliners_num, inliners_score,
		keypoints_num, keypoints_score,
		pic_data, pic_length,
		pPointDataset, pMatDataset);
}

extern "C" dave_bool
dave_opencv_matcher_on_image_path(
	ub *inliners_num, float *inliners_score,
	ub *keypoint_num, float *keypoint_score,
	s8 *image_path_a, s8 *image_path_b,
	ub nfeatures)
{
	return matcher_sift_on_image_path(
		inliners_num, inliners_score,
		keypoint_num, keypoint_score,
		image_path_a, image_path_b,
		nfeatures);
}

extern "C" void
dave_opencv_surf(s8 *image_path, ub *time)
{
	features_surf(image_path, time);
}

extern "C" dave_bool
__dave_opencv_matrix_malloc__(Matrix *pMatrix, s8 *pic_path, u8 *pic_data, ub pic_length, s8 *fun, ub line)
{
	return __imread_matrix_malloc__(pMatrix, pic_path, pic_data, pic_length, fun, line);
}

extern "C" void
dave_opencv_matrix_free(Matrix *pMatrix)
{
	return imread_matrix_free(pMatrix);
}

extern "C" dave_bool
dave_opencv_sift_valid_size(ub size)
{
	if((size >= 5) && (size <= 2000))
	{
		return dave_true;
	}
	else
	{
		return dave_false;
	}
}

extern "C" ub
dave_opencv_mat_to_nfeatures(OpenCVMat *pMat)
{
	ub nfeatures = (ub)(pMat->rows);

	if((nfeatures != 500) && (nfeatures != 1000))
	{
		PARTYDEBUG("unexpected parameters nfeatures:%d", nfeatures);
	}

	// only support 500 and 1000 features.

	if(nfeatures >= 1000)
	{
		nfeatures = 1000;
	}
	else if(nfeatures >= 500)
	{
		nfeatures = 500;
	}
	else
	{
		nfeatures = 500;
	}

	return nfeatures;
}

#endif


/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __FEATURES_SIFT_HPP__
#define __FEATURES_SIFT_HPP__

extern "C" dave_bool features_sift(u8 *pic_data, ub pic_length, ub nfeatures, CVKeyPoint *pPoint, OpenCVMat *pMat);

extern "C" ub matcher_sift(
	ub opt_number,
	ub *inliners_num, float *inliners_score,
	ub *keypoints_num, float *keypoints_score,
	u8 *pic_data, ub pic_length,
	CVKeyPoint *pPointDataset, OpenCVMat *pMatDataset);

extern "C" dave_bool matcher_sift_on_image_path(
	ub *inliners_num, float *inliners_score,
	ub *keypoint_num, float *keypoint_score,
	s8 *image_path_a, s8 *image_path_b,
	ub nfeatures);

#endif


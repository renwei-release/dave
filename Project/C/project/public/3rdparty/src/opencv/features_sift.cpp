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
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/xfeatures2d/nonfree.hpp"
#include "opencv2/calib3d/calib3d_c.h"
#include "opencv2/imgproc/types_c.h"
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
#include "opencv_magsac.hpp"
#include "opencv_ransac.hpp"
#include "party_log.h"

extern "C" ub dave_opencv_mat_to_nfeatures(OpenCVMat *pMat);
extern "C" dave_bool dave_opencv_sift_valid_size(ub size);

#define SIFT_RESIZE_ROWS 500
#define SIFT_RESIZE_COLS 500

// #define TEST_DATA_CONVERSION
// #define SIFT_ENABLE_CLAHE
 #define SIFT_ENABLE_CUT

using namespace cv;
using namespace std;
using namespace cv::xfeatures2d;

static void
_features_sift_store(CVKeyPoint *pPoint, OpenCVMat *pMat, std::vector<cv::KeyPoint> &keypoints, Mat &descriptors, int width, int height)
{
	_KeyPoint_ *pPointData;
	ub point_index;
	ub mat_ptr_length;

	pPoint->width = width;
	pPoint->height = height;
	pPoint->size = keypoints.size();
	pPoint->_KeyPoint_ = dave_mmalloc(sizeof(_KeyPoint_) * pPoint->size);
	pPointData = (_KeyPoint_ *)(pPoint->_KeyPoint_->payload);
	for(point_index=0; point_index<(ub)(pPoint->size); point_index++)
	{
		pPointData[point_index].pt_x = keypoints[point_index].pt.x;
		pPointData[point_index].pt_y = keypoints[point_index].pt.y;
		pPointData[point_index].size = keypoints[point_index].size;
		pPointData[point_index].angle = keypoints[point_index].angle;
		pPointData[point_index].response = keypoints[point_index].response;
		pPointData[point_index].octave = keypoints[point_index].octave;
		pPointData[point_index].class_id = keypoints[point_index].class_id;
	}

	pMat->type = descriptors.type();
	pMat->flags = descriptors.flags;
	pMat->dims = descriptors.dims;
	pMat->rows = descriptors.rows;
	pMat->cols = descriptors.cols;
	mat_ptr_length = mat_to_data_length(descriptors);
	pMat->Mat = dave_mmalloc(mat_ptr_length);
	dave_memcpy(pMat->Mat->payload, descriptors.ptr(0, 0), mat_ptr_length);

	PARTYDEBUG("keypoint:%d mat:%d", pPoint->_KeyPoint_->len, pMat->Mat->len);
}

static dave_bool
_features_build_load(std::vector<cv::KeyPoint> &keypoints, Mat &descriptors, CVKeyPoint *pPoint, OpenCVMat *pMat)
{
	int key_index;
	_KeyPoint_ *pKeyPoint;
	KeyPoint kpt;
	dave_bool point_flag, mat_flag;

	PARTYDEBUG("_KeyPoint_:%d mat:%d", pPoint->_KeyPoint_->len, pMat->Mat->len);

	keypoints.clear();
	descriptors.release();

	point_flag = mat_flag = dave_false;

	if(pPoint != NULL)
	{
		if((pPoint->_KeyPoint_ != NULL) && (dave_opencv_sift_valid_size((ub)(pPoint->size)) == dave_true))
		{
			keypoints.resize(0);
			keypoints.reserve((int)(pPoint->size));
			pKeyPoint = (_KeyPoint_ *)(pPoint->_KeyPoint_->payload);
			for(key_index=0; key_index<pPoint->size; key_index++)
			{
				kpt.pt.x = pKeyPoint[key_index].pt_x;
				kpt.pt.y = pKeyPoint[key_index].pt_y;
				kpt.size = pKeyPoint[key_index].size;
				kpt.angle = pKeyPoint[key_index].angle;
				kpt.response = pKeyPoint[key_index].response;
				kpt.octave = pKeyPoint[key_index].octave;
				kpt.class_id = pKeyPoint[key_index].class_id;

				keypoints.push_back(kpt);
			}

			point_flag = dave_true;
		}
	}

	if(pMat != NULL)
	{
		if(pMat->Mat != NULL)
		{
			descriptors.create(pMat->rows, pMat->cols, pMat->type);
			descriptors.flags = pMat->flags;
			descriptors.dims = pMat->dims;
			descriptors.rows = pMat->rows;
			descriptors.cols = pMat->cols;
			dave_memcpy(descriptors.ptr(0, 0), pMat->Mat->payload, pMat->Mat->len);

			mat_flag = dave_true;
		}
	}

	if((point_flag == dave_true) && (mat_flag == dave_true))
	{
		return dave_true;
	}
	else
	{
		return dave_false;
	}
}

static void
_features_test_data_conversion(OpenCVMat *pMat, Mat &descriptors)
{
#ifdef TEST_DATA_CONVERSION
	ub mat_ptr_length;
	std::vector<cv::KeyPoint> test_keypoints;
	Mat test_descriptors;

	mat_ptr_length = mat_to_data_length(descriptors);

	if(_features_build_load(test_keypoints, test_descriptors, NULL, pMat) == dave_true)
	{
		if(dave_memcmp(descriptors.ptr(0, 0), test_descriptors.ptr(0, 0), mat_ptr_length) == dave_false)
		{
			PARTYABNOR("invalid data! mat_ptr_length:%d", mat_ptr_length);
		}
	}
	else
	{
		PARTYABNOR("load features failed!");
	}
#endif
}

#ifdef SIFT_ENABLE_CUT

static dave_bool
_features_shape_auto_cut(int pic_width, int pic_height, int reference_width, int reference_height)
{
	float pic_proportion, reference_proportion, gap_proportion;

	if((pic_width == 0) || (pic_height == 0) || (reference_width == 0) || (reference_height == 0))
	{
		PARTYLOG("empty pic! %d,%d,%d,%d",
			pic_width, pic_height, reference_width, reference_height);
		return dave_false;
	}

	pic_proportion = (float)pic_width/(float)pic_height;
	reference_proportion = (float)reference_width/(float)reference_height;

	if(pic_proportion > reference_proportion)
	{
		gap_proportion = pic_proportion - reference_proportion;
	}
	else
	{
		gap_proportion = reference_proportion - pic_proportion;
	}

	PARTYDEBUG("pic:%d,%d,%lf reference:%d,%d,%lf gap:%lf",
		pic_width, pic_height, pic_proportion,
		reference_width, reference_height, reference_proportion,
		gap_proportion);

	if(gap_proportion < 0.7)
	{
		return dave_false;
	}

	return dave_true;
}

static void
_features_sift_process_cut(Mat &grayscale_image, int reference_width, int reference_height)
{
	int image_width, image_height;
	float reference_ratio;
	cv::Rect maxRect;

	image_width = grayscale_image.cols;
	image_height = grayscale_image.rows;

	reference_ratio = (float)reference_width / (float)reference_height;

	image_width = (int)(reference_ratio * (float)image_height);
	if(image_width > grayscale_image.cols)
	{
		image_width = grayscale_image.cols;

		image_height = (int)((float)image_width / reference_ratio);
		if(image_height > grayscale_image.rows)
		{			
			image_height = grayscale_image.rows;
		}
	}

	if((image_width == grayscale_image.cols) && (image_height == grayscale_image.rows))
	{
		return;
	}

	maxRect.x = (grayscale_image.cols / 2) - (image_width / 2);
	if(maxRect.x < 0)
	{
		maxRect.x = 0;
	}
	maxRect.y = (grayscale_image.rows / 2) - (image_height / 2);
	if(maxRect.y < 0)
	{
		maxRect.y = 0;
	}
	maxRect.width = image_width;
	maxRect.height = image_height;

	if(((maxRect.x + maxRect.width) > grayscale_image.cols)
		|| ((maxRect.y + maxRect.height) > grayscale_image.rows))
	{
		return;
	}

	PARTYDEBUG("maxRect:%d,%d,%d,%d grayscale_image:%d,%d,%lf reference:%d,%d,%lf <%lf %lf>",
		maxRect.x, maxRect.y, maxRect.width, maxRect.height,
		grayscale_image.cols, grayscale_image.rows, (float)(grayscale_image.cols)/(float)(grayscale_image.rows),
		reference_width, reference_height, (float)reference_width/(float)reference_height,
		(float)(grayscale_image.cols)/(float)(reference_width),
		(float)grayscale_image.rows/(float)reference_height);

	Mat image_roi = grayscale_image(maxRect);

	image_roi.copyTo(grayscale_image);
}

#else

static dave_bool
_features_shape_auto_cut(int pic_width, int pic_height, int reference_width, int reference_height)
{
	return dave_false;
}

#endif

#ifdef SIFT_ENABLE_CLAHE

static void
_features_sift_process_clahe(Mat &image_read, Mat &grayscale_image)
{
	/*
	 * https://answers.opencv.org/question/12024/use-of-clahe/
	 * https://www.cnblogs.com/Imageshop/archive/2013/04/07/3006334.html
	 * 限制对比度自适应直方图均衡（Contrast Limited Adaptive histgram equalization/CLAHE)
	 * CLAHE主要是用来克服AHE的过度放大噪音的问题。
	 * 可以保证图片能提取到足够特征。
	 */
	Mat clahe_dst;
	Ptr<CLAHE> clahe = createCLAHE();
	clahe->setClipLimit(10);
	clahe->apply(grayscale_image, clahe_dst);

	resize(clahe_dst, image_read, Size(SIFT_RESIZE_ROWS, SIFT_RESIZE_COLS));
}

#endif

static dave_bool
_features_sift_process_read(Mat &image_read, Mat &grayscale_image, int reference_width, int reference_height, dave_bool auto_cut)
{
	if(grayscale_image.empty())
	{
		PARTYABNOR("grayscale_image read error!");
		return dave_false;
	}
	if(grayscale_image.data == NULL)
	{
		PARTYABNOR("grayscale_image read error!");
		return dave_false;
	}

#ifdef SIFT_ENABLE_CUT
	if(auto_cut == dave_true)
	{
		_features_sift_process_cut(grayscale_image, reference_width, reference_height);
	}
#endif

#ifdef SIFT_ENABLE_CLAHE
	_features_sift_process_clahe(image_read, grayscale_image);
#else
	resize(grayscale_image, image_read, Size(SIFT_RESIZE_ROWS, SIFT_RESIZE_COLS));
#endif

	return dave_true;
}

static dave_bool
_features_sift_image_read(Mat &image_read, s8 *image_path, int *width, int *height, int reference_width, int reference_height, dave_bool auto_cut)
{
	Mat grayscale_image;
	dave_bool ret;

	grayscale_image = imread((const char *)image_path, IMREAD_GRAYSCALE);

	ret = _features_sift_process_read(image_read, grayscale_image, reference_width, reference_height, auto_cut);

	if(width != NULL)
	{
		*width = grayscale_image.cols;
	}
	if(height != NULL)
	{
		*height = grayscale_image.rows;
	}

	return ret;
}

static dave_bool
_features_sift_ram_read(Mat &image_read, u8 *pic_data, ub pic_length, int *width, int *height, int reference_width, int reference_height, dave_bool auto_cut)
{
	Mat grayscale_image;
	dave_bool ret;

	if((pic_data == NULL) || (pic_length == 0))
	{
		PARTYABNOR("pic_data:%x or pic_length:%d", pic_data, pic_length);
		return dave_false;
	}

	_InputArray pic_arr(pic_data, pic_length);

	grayscale_image = imdecode(pic_arr, IMREAD_GRAYSCALE);

	ret = _features_sift_process_read(image_read, grayscale_image, reference_width, reference_height, auto_cut);

	if(width != NULL)
	{
		*width = grayscale_image.cols;
	}
	if(height != NULL)
	{
		*height = grayscale_image.rows;
	}

	return ret;
}

static dave_bool
_features_knnMatch(vector<DMatch> &matches, Mat &mat_a, Mat &mat_b)
{
	const int k = 2;
	float distanceRatio;
	const float ratio_distance = 0.6;
	const int min_match_size = 4;
	cv::Ptr<cv::flann::IndexParams> indexParams = cv::makePtr<cv::flann::KDTreeIndexParams>(5);
	cv::Ptr<cv::flann::SearchParams> searchParams = cv::makePtr<cv::flann::SearchParams>(50);

	FlannBasedMatcher matcher(indexParams, searchParams);
	vector<vector<DMatch>> knnMatches;
	size_t knn_size;

	matcher.knnMatch(mat_a, mat_b, knnMatches, k);

	knn_size = knnMatches.size();

	for(size_t i=0; i<knn_size; i++)
	{
		const DMatch& bestMatch = knnMatches[i][0];			// k = 0
		const DMatch& betterMatch = knnMatches[i][1];		// k = 1

		if(bestMatch.distance < betterMatch.distance)
		{
			distanceRatio = bestMatch.distance / betterMatch.distance;
		}
		else
		{
			distanceRatio = betterMatch.distance / bestMatch.distance;
		}

		if(distanceRatio <= ratio_distance)
		{
			PARTYDEBUG("%d %lf %lf %lf", i, bestMatch.distance, betterMatch.distance, distanceRatio);

			matches.push_back(bestMatch);
		}
	}

	PARTYDEBUG("knnMatch size:%d/%d/%d", knn_size, matches.size(), min_match_size);

	if(matches.size() > min_match_size)
	{
		return dave_true;
	}
	else
	{
		return dave_false;
	}
}

static dave_bool
_features_matcher(
	ub *inliners_num, float *inliners_score,
	ub *keypoint_num, float *keypoint_score,
	std::vector<cv::KeyPoint> &keypoints_a, Mat &descriptors_a,
	std::vector<cv::KeyPoint> &keypoints_b, Mat &descriptors_b)
{
	vector<DMatch> matches;
	ub inliners_size, inliners_match_size;
	dave_bool ret;

	if(_features_knnMatch(matches, descriptors_a, descriptors_b) == dave_true)
	{
		magsac_findHomography(&inliners_size, &inliners_match_size, matches, keypoints_a, keypoints_b);

		if(inliners_match_size > 0)
		{
			*inliners_num = inliners_size;
			if(*inliners_num == 0)
			{
				PARTYLOG("find the inliners_size is zero, why? %d", inliners_match_size);
				*inliners_score = 0;
			}
			else
			{
				*inliners_score = (float)inliners_match_size / (float)inliners_size;
			}
			*keypoint_num = matches.size();
			if(*keypoint_num == 0)
			{
				PARTYLOG("find the matches.size is zero, why? %d", inliners_match_size);
				*keypoint_score = 0;
			}
			else
			{
				*keypoint_score = (float)inliners_match_size / (float)(*keypoint_num);
			}

			PARTYDEBUG("inliners(%d/%d/%lf) keypoints(%d/%d/%lf)",
				inliners_match_size, inliners_size, *inliners_score,
				inliners_match_size, matches.size(), *keypoint_score);

			ret = dave_true;
		}
		else
		{
			ret = dave_false;
		}
	}
	else
	{
		PARTYDEBUG("knn not match!");
		ret = dave_false;
	}

	return ret;
}

static void
_features_sift_rootsift(Mat &rootSiftFeature, Mat &siftFeature)
{
	// https://www.cnblogs.com/wangguchangqing/p/9471103.html
	int rows_index;

	for(rows_index=0; rows_index<siftFeature.rows; rows_index++)
	{
        // Conver to float type
        Mat f;

		siftFeature.row(rows_index).convertTo(f, CV_32FC1);

		normalize(f, f, 1, 0, NORM_L1); // l1 normalize
		sqrt(f, f); // sqrt-root  root-sift

		rootSiftFeature.push_back(f);
	}
}

static dave_bool
_features_sift_features_(
	s8 *pic_path, u8 *pic_data, ub pic_length, ub nfeatures,
	std::vector<cv::KeyPoint> &keypoints, Mat &descriptors,
	int *width, int *height,
	int reference_width, int reference_height,
	dave_bool auto_cut)
{
	Mat image_read;
	cv::Ptr<Feature2D> sift = cv::xfeatures2d::SIFT::create((int)nfeatures);
	Mat detectAndCompute_descriptors;

	keypoints.clear();
	descriptors.release();

	if(pic_path != NULL)
	{
		if(_features_sift_image_read(image_read, pic_path, width, height, reference_width, reference_height, auto_cut) == dave_false)
		{
			return dave_false;
		}
	}
	else
	{
		if(_features_sift_ram_read(image_read, pic_data, pic_length, width, height, reference_width, reference_height, auto_cut) == dave_false)
		{
			return dave_false;
		}
	}

	sift->detectAndCompute(image_read, Mat(), keypoints, detectAndCompute_descriptors, false);

	if(keypoints.size() <= 0)
	{
		PARTYDEBUG("image:%s has empty keypoints!", pic_path);
		return dave_false;
	}

	_features_sift_rootsift(descriptors, detectAndCompute_descriptors);

	return dave_true;
}

static dave_bool
_features_sift_features(u8 *pic_data, ub pic_length, ub nfeatures, CVKeyPoint *pPoint, OpenCVMat *pMat)
{
	std::vector<cv::KeyPoint> keypoints;
	Mat descriptors;
	int width, height;
	dave_bool ret;

	if(_features_sift_features_(NULL, pic_data, pic_length, nfeatures, keypoints, descriptors, &width, &height, 0, 0, dave_false) == dave_false)
	{
		return dave_false;
	}

	_features_sift_store(pPoint, pMat, keypoints, descriptors, width, height);

	if(keypoints.size() <= 0)
	{
		PARTYLOG("nfeatures:%d keypoint size:%d descriptors rows:%d cols:%d",
			nfeatures,
			keypoints.size(),
			descriptors.rows, descriptors.cols);

		dave_mfree(pPoint->_KeyPoint_); pPoint->_KeyPoint_ = NULL;
		dave_mfree(pMat->Mat); pMat->Mat = NULL;
		ret = dave_false;
	}
	else
	{
		_features_test_data_conversion(pMat, descriptors);
		ret = dave_true;
	}

	keypoints.clear();
	descriptors.release();

	return ret;
}

static dave_bool
_matcher_sift_on_image_path(
	ub *inliners_num, float *inliners_score,
	ub *keypoint_num, float *keypoint_score,
	s8 *image_path_a, s8 *image_path_b, ub nfeatures)
{
	std::vector<cv::KeyPoint> keypoints_a;
	std::vector<cv::KeyPoint> keypoints_b;
	Mat descriptors_a;
	Mat descriptors_b;

	_features_sift_features_(image_path_a, NULL, 0, nfeatures, keypoints_a, descriptors_a, NULL, NULL, 0, 0, dave_false);
	_features_sift_features_(image_path_b, NULL, 0, nfeatures, keypoints_b, descriptors_b, NULL, NULL, 0, 0, dave_false);

	return _features_matcher(inliners_num, inliners_score, keypoint_num, keypoint_score, keypoints_a, descriptors_a, keypoints_b, descriptors_b);
}

static dave_bool
_matcher_sift(
	u8 *pic_data, ub pic_length, ub nfeatures,
	int original_width, int original_height,
	std::vector<cv::KeyPoint> &keypoint_original_pic,
	Mat &descriptor_original_pic,
	ub opt_index,
	ub *inliners_num, float *inliners_score,
	ub *keypoints_num, float *keypoints_score,
	CVKeyPoint *pPointDataset, OpenCVMat *pMatDataset)
{
	dave_bool ret, auto_cut;
	std::vector<cv::KeyPoint> keypoint_cut_pic;
	Mat descriptor_cut_pic;
	std::vector<cv::KeyPoint> keypoint_dataset;
	Mat descriptor_dataset;

	auto_cut = _features_shape_auto_cut(original_width, original_height, pPointDataset[opt_index].width, pPointDataset[opt_index].height);
	if(auto_cut == dave_false)
	{
		ret = dave_true;
	}
	else
	{
		ret = _features_sift_features_(
			NULL, pic_data, pic_length, nfeatures,
			keypoint_cut_pic, descriptor_cut_pic,
			NULL, NULL,
			pPointDataset[opt_index].width, pPointDataset[opt_index].height,
			dave_true);
	}

	if(ret == dave_true)
	{
		ret = _features_build_load(keypoint_dataset, descriptor_dataset, &pPointDataset[opt_index], &pMatDataset[opt_index]);
	}
	
	if(ret == dave_true)
	{
		if(auto_cut == dave_false)
		{
			ret = _features_matcher(
				&inliners_num[opt_index], &inliners_score[opt_index],
				&keypoints_num[opt_index], &keypoints_score[opt_index],
				keypoint_original_pic, descriptor_original_pic,
				keypoint_dataset, descriptor_dataset);
		}
		else
		{
			ret = _features_matcher(
				&inliners_num[opt_index], &inliners_score[opt_index],
				&keypoints_num[opt_index], &keypoints_score[opt_index],
				keypoint_cut_pic, descriptor_cut_pic,
				keypoint_dataset, descriptor_dataset);
		}
	}
	
	if(ret == dave_false)
	{
		inliners_num[opt_index] = 0;
		inliners_score[opt_index] = 0;
		keypoints_num[opt_index] = 0;
		keypoints_score[opt_index] = 0; 		
	}

	keypoint_dataset.clear();
	descriptor_dataset.release();
	keypoint_cut_pic.clear();
	descriptor_cut_pic.release();

	return ret;
}

// =====================================================================

extern "C" dave_bool
features_sift(u8 *pic_data, ub pic_length, ub nfeatures, CVKeyPoint *pPoint, OpenCVMat *pMat)
{
	dave_bool ret;

	PARTYDEBUG("nfeatures:%d", nfeatures);

	pPoint->size = 0;
	pPoint->_KeyPoint_ = NULL;
	pMat->type = 0;
	pMat->flags = 0;
	pMat->dims = 0;
	pMat->rows = 0;
	pMat->cols = 0;
	pMat->Mat = NULL;

	if((pic_data == NULL) || (pic_length == 0))
	{
		PARTYABNOR("invalid pic_data:%x or pic_length:%d", pic_data, pic_length);
		return dave_false;
	}

	ret = _features_sift_features(pic_data, pic_length, nfeatures, pPoint, pMat);

	if((ret == dave_false) || (dave_opencv_sift_valid_size((ub)(pPoint->size)) == dave_false))
	{
		dave_mfree(pPoint->_KeyPoint_); pPoint->_KeyPoint_ = NULL;
		dave_mfree(pMat->Mat); pMat->Mat = NULL;

		pPoint->size = 0;
		pMat->type = 0;
		pMat->flags = 0;
		pMat->dims = 0;
		pMat->rows = 0;
		pMat->cols = 0;

		ret = dave_false;
	}

	return ret;
}

extern "C" ub
matcher_sift(
	ub opt_number,
	ub *inliners_num, float *inliners_score,
	ub *keypoints_num, float *keypoints_score,
	u8 *pic_data, ub pic_length,
	CVKeyPoint *pPointDataset, OpenCVMat *pMatDataset)
{
	dave_bool *valid_array;
	ub nfeatures = dave_opencv_mat_to_nfeatures(&pMatDataset[0]);
	ub opt_index, valid_number;
	std::vector<cv::KeyPoint> keypoint_original_pic;
	Mat descriptor_original_pic;
	int original_width, original_height;
	dave_bool ret;

	ret = _features_sift_features_(
		NULL, pic_data, pic_length, nfeatures,
		keypoint_original_pic, descriptor_original_pic,
		&original_width, &original_height,
		0, 0,
		dave_false);

	if(ret == dave_false)
	{
		return 0;
	}

	valid_array = (dave_bool *)dave_malloc(opt_number);
	for(opt_index=0; opt_index<opt_number; opt_index++)
	{
		valid_array[opt_index] = dave_false;
	}

#pragma omp parallel for num_threads(opt_number)
	for(opt_index=0; opt_index<opt_number; opt_index++)
	{
		if(_matcher_sift(
			pic_data, pic_length, nfeatures,
			original_width, original_height,
			keypoint_original_pic,
			descriptor_original_pic,
			opt_index,
			inliners_num, inliners_score,
			keypoints_num, keypoints_score,
			pPointDataset, pMatDataset) == dave_true)
		{
			valid_array[opt_index] = dave_true;
		}
	}

	valid_number = 0;

	for(opt_index=0; opt_index<opt_number; opt_index++)
	{
		if(valid_array[opt_index] == dave_true)
		{
			valid_number ++;
		}
	}

	keypoint_original_pic.clear();
	descriptor_original_pic.release();

	return valid_number;
}

extern "C" dave_bool
matcher_sift_on_image_path(
	ub *inliners_num, float *inliners_score,
	ub *keypoint_num, float *keypoint_score,
	s8 *image_path_a, s8 *image_path_b,
	ub nfeatures)
{
	dave_bool ret;

	*inliners_num = 0;
	*inliners_score = 0;
	*keypoint_num = 0;
	*keypoint_score = 0;

	ret = _matcher_sift_on_image_path(inliners_num, inliners_score, keypoint_num, keypoint_score, image_path_a, image_path_b, nfeatures);

	return ret;
}

#endif


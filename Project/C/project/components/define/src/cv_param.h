/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __CV_PARAM_H__
#define __CV_PARAM_H__
#include "dave_enum.h"
#include "dave_parameters.h"
#include "dave_mcard.h"

#define DEFAULT_RECTANGLE_ID (0)

#define IMAGE_FEATURE_DB_NAME (s8 *)"image_feature_table_v2"

typedef enum {
	MatrixOpt_none,
	MatrixOpt_base,
	MatrixOpt_vgg,
	MatrixOpt_centernet,
	MatrixOpt_unet,
	MatrixOpt_traffic,
	MatrixOpt_max = 0xffffffffffffffff
} MatrixOpt;

typedef struct {
	float pt_x;
	float pt_y;
	float size;
	float angle;
	float response;
	int octave;
	int class_id;
} _KeyPoint_;

typedef struct {
	int width;
	int height;
	int size;
	MBUF *_KeyPoint_;
} CVKeyPoint;

typedef struct {
	int type;
	int flags;
	int dims;
	int rows;
	int cols;
	MBUF *Mat;
} OpenCVMat;

typedef struct {
	float x1;
	float y1;
	float x2;
	float y2;

	float w;
	float h;
} Rectangle;

typedef struct {
	ub matrix_row;
	ub matrix_column;
	ub matrix_depth;

	DaveDataType matrix_type;
	ub matrix_length;
	void *matrix_data;

	ub dime1_depth;
	ub dime2_depth;
	ub dime3_depth;

	ub raw_w;
	ub raw_h;
	ub raw_data_length;
	u8 *raw_data_ptr;

	DaveDataType extra_type;
	ub extra_length;
	void *extra_data;

	ub feature_length;
	float *feature_data;

	Rectangle rectangle;
	float score;
} MatrixData;

typedef struct {
	MatrixOpt opt;

	MatrixData original;
	MatrixData net;
} Matrix;

typedef enum {
	CVSearchOpt_0,
	CVSearchOpt_1,
	CVSearchOpt_2,
	CVSearchOpt_max
} CVSearchOpt;

typedef struct {
	CVSearchOpt search_opt;
	MCardContentType content_type;
	LanguageCode language_code;
	s8 image_local_path[DAVE_PATH_LEN];
	s8 image_url_path[DAVE_PATH_LEN];

	ub opt_number;
	ub face_number;
	s8 vgg_id[DAVE_KEY_OPT_MAX][DAVE_SHA1_IMAGE_ID];
	float vgg_score[DAVE_KEY_OPT_MAX];
	Rectangle rectangle[DAVE_KEY_OPT_MAX];
	s8 image_title[DAVE_KEY_OPT_MAX][DAVE_IMAGE_TITLE_LEN];
	s8 image_painter[DAVE_KEY_OPT_MAX][DAVE_USER_NAME_LEN];
	ub inliners_num[DAVE_KEY_OPT_MAX];
	float inliners_score[DAVE_KEY_OPT_MAX];
	ub keypoints_num[DAVE_KEY_OPT_MAX];
	float keypoints_score[DAVE_KEY_OPT_MAX];

	dave_bool confidence;
	s8 label[DAVE_LABEL_STR_MAX];
	float score;

	ub cnn_model_work_time;
	ub features_db_req_time;
	ub features_db_rsp_time;
	ub features_db_process_time;
	ub introduce_db_req_time;
	ub introduce_db_rsp_time;

	MBUF *model_raw_data;
} CVModelResult;

typedef struct {
	CVModelResult model_result;
	ImageIntroduction image_introduction;
} CVResult;

#endif


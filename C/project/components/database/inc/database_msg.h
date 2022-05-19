/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DATABASE_MSG_H__
#define __DATABASE_MSG_H__
#include "dave_base.h"
#include "dave_enum.h"
#include "dave_parameters.h"
#include "dave_struct.h"
#include "dave_error_code.h"
#include "dave_mcard.h"
#include "cv_param.h"

#define DATABASE_THREAD_NAME "dba_DATABASE"

typedef enum {	
	REQ_TYPE_MAX		 = 0x1fffffff
} ReqType;

/* for DBMSG_SYS_INQ_CHANNEL_REQ message */
typedef struct {
	s8 channel_name[DAVE_NORMAL_NAME_LEN];
	ub table_id;
	void *ptr;
} DBSysInqChannelReq;

/* for DBMSG_SYS_INQ_CHANNEL_RSP message */
typedef struct {
	ErrCode ret;
	dave_bool valid_flag;
	s8 db_name[DAVE_NORMAL_NAME_LEN];
	s8 channel_name[DAVE_NORMAL_NAME_LEN];
	ub table_id;
	s8 password[DAVE_PASSWORD_LEN];
	s8 auth_key_str[DAVE_AUTH_KEY_STR_LEN];
	ChannelInfo channel_info;
	DateStruct validity_date;
	ub next_store_table_id;
	s8 uip_cmd_str[DAVE_UIP_CMD_STR_LEN];
	s8 forbidden_uip_cmd_str[DAVE_UIP_CMD_STR_LEN];
	void *ptr;
} DBSysInqChannelRsp;

/* for DBMSG_SYS_INQ_IMAGE_REQ message */
typedef struct {
	s8 image_id[DAVE_SHA1_IMAGE_ID];
	LanguageCode language_code;
	dave_bool details;
	void *ptr;
} DBSysInqImageReq;

/* for DBMSG_SYS_INQ_IMAGE_RSP message */
typedef struct {
	ErrCode ret;
	s8 image_id[DAVE_SHA1_IMAGE_ID];
	ImageIntroduction image;
	void *ptr;
} DBSysInqImageRsp;

/* for DBMSG_SYS_INQ_MUSEUM_REQ message */
typedef struct {
	s8 museum_id[DAVE_SHA1_IMAGE_ID];
	LanguageCode language_code;
	dave_bool details;
	void *ptr;
} DBSysInqMuseumReq;

/* for DBMSG_SYS_INQ_MUSEUM_RSP message */
typedef struct {
	ErrCode ret;
	s8 museum_id[DAVE_SHA1_IMAGE_ID];
	MuseumIntroduction museum;
	void *ptr;
} DBSysInqMuseumRsp;

/* for DBMSG_SYS_INQ_MUSEUM_PAGE_REQ message */
typedef struct {
	ub table_id;
	ub page_id;
	ub page_number;
	void *ptr;
} DBSysInqMuseumPageReq;

/* for DBMSG_SYS_INQ_MUSEUM_PAGE_RSP message */
typedef struct {
	ErrCode ret;
	s8 museum_id[DAVE_SHA1_IMAGE_ID];
	ub table_id;
	ub page_id;
	ub total_number;
	ub page_number;
	s8 image_id_page[DAVE_DBA_PAGE_MAX][DAVE_SHA1_IMAGE_ID];
	void *ptr;
} DBSysInqMuseumPageRsp;

/* for DBMSG_SYS_ADD_WEICHAT_REQ message */
typedef struct {
	WeiChatUserInfo info;
	void *ptr;
} DBSysAddWeiChatReq;

/* for DBMSG_SYS_ADD_WEICHAT_RSP message */
typedef struct {
	ErrCode ret;
	void *ptr;
} DBSysAddWeiChatRsp;

/* for DBMSG_SYS_INQ_WEICHAT_REQ message */
typedef struct {
	s8 uuid[64];
	s8 openid[64];
	void *ptr;
} DBSysInqWeiChatReq;

/* for DBMSG_SYS_INQ_WEICHAT_RSP message */
typedef struct {
	ErrCode ret;
	ub table_id;
	WeiChatUserInfo info;
	void *ptr;
} DBSysInqWeiChatRsp;

/* for DBMSG_SYS_INQ_PAINTER_PAGE_REQ message */
typedef struct {
	ub table_id;
	ub page_id;
	ub page_number;
	void *ptr;
} DBSysInqPainterPageReq;

/* for DBMSG_SYS_INQ_PAINTER_PAGE_RSP message */
typedef struct {
	ErrCode ret;
	s8 painter_id[DAVE_SHA1_IMAGE_ID];
	ub table_id;
	ub page_id;
	ub total_number;
	ub page_number;
	s8 image_id_page[DAVE_DBA_PAGE_MAX][DAVE_SHA1_IMAGE_ID];
	void *ptr;
} DBSysInqPainterPageRsp;

/* for DBMSG_NOSQL_ADD_TALK_REQ message */
typedef struct {
	MBUF *portal_data;
	MCard from_client;
	MCard from_server;
	UniversalLabel label;
	MBUF *model_raw_data;
	void *ptr;
} DBNosqlAddTalkReq;

/* for DBMSG_NOSQL_ADD_TALK_RSP message */
typedef struct {
	ErrCode ret;
	void *ptr;
} DBNosqlAddTalkRsp;

/* for DBMSG_REDIS_DEL_TABLE_REQ message */
typedef struct {
	ReqType req_type;
	s8 table_name[DAVE_NORMAL_NAME_LEN];
	void *ptr;
} DBRedisDelTableReq;

/* for DBMSG_REDIS_DEL_TABLE_RSP message */
typedef struct {
	ReqType req_type;
	ErrCode ret;
	s8 table_name[DAVE_NORMAL_NAME_LEN];
	void *ptr;
} DBRedisDelTableRsp;

/* for DBMSG_SYS_ADD_IMAGE_FEATURE_REQ message */
typedef struct {
	s8 table_name[DAVE_NORMAL_NAME_LEN];
	s8 image_id[DAVE_SHA1_IMAGE_ID];
	CVKeyPoint point;
	OpenCVMat mat;
	float vgg_feature[DAVE_VGG_FEATURE_LEN];
	void *ptr;
} DBSysAddImageFeatureReq;

/* for DBMSG_SYS_ADD_IMAGE_FEATURE_RSP message */
typedef struct {
	ErrCode ret;
	s8 table_name[DAVE_NORMAL_NAME_LEN];
	s8 image_id[DAVE_SHA1_IMAGE_ID];
	void *ptr;
} DBSysAddImageFeatureRsp;

/* for DBMSG_SYS_INQ_IMAGE_FEATURE_REQ message */
typedef struct {
	s8 table_name[DAVE_NORMAL_NAME_LEN];
	ub table_id;
	s8 image_id[DAVE_KEY_OPT_MAX][DAVE_SHA1_IMAGE_ID];
	void *ptr;
} DBSysInqImageFeatureReq;

/* for DBMSG_SYS_INQ_IMAGE_FEATURE_RSP message */
typedef struct {
	ErrCode ret;
	ub process_number;
	s8 table_name[DAVE_NORMAL_NAME_LEN];
	ub table_id;
	s8 image_id[DAVE_KEY_OPT_MAX][DAVE_SHA1_IMAGE_ID];
	CVKeyPoint point[DAVE_KEY_OPT_MAX];
	OpenCVMat mat[DAVE_KEY_OPT_MAX];
	float vgg_feature[DAVE_VGG_FEATURE_LEN];
	ub process_time;
	void *ptr;
} DBSysInqImageFeatureRsp;

/* for DBMSG_HYBRID_ADD_LIST_REQ message */
typedef struct {
	s8 table[DAVE_NORMAL_NAME_LEN];
	s8 key[DAVE_KEY_LEN_MAX];
	MBUF *value;
	void *ptr;
} DBHybridAddListReq;

/* for DBMSG_HYBRID_ADD_LIST_RSP message */
typedef struct {
	ErrCode ret;
	s8 table[DAVE_NORMAL_NAME_LEN];
	s8 key[DAVE_KEY_LEN_MAX];
	void *ptr;
} DBHybridAddListRsp;

/* for DBMSG_HYBRID_INQ_LIST_REQ message */
typedef struct {
	dave_bool direct;
	s8 table[DAVE_NORMAL_NAME_LEN];
	s8 key[DAVE_KEY_LEN_MAX];
	ub page_id;
	ub page_number;
	void *ptr;
} DBHybridInqListReq;

/* for DBMSG_HYBRID_INQ_LIST_RSP message */
typedef struct {
	ErrCode ret;
	dave_bool direct;
	s8 table[DAVE_NORMAL_NAME_LEN];
	s8 key[DAVE_KEY_LEN_MAX];
	ub page_id;
	ub total_number;
	ub page_number;
	MBUF *value;
	void *ptr;
} DBHybridInqListRsp;

#endif


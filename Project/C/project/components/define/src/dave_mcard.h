/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_MCARD_H__
#define __DAVE_MCARD_H__
#include "dave_parameters.h"
#include "dave_struct.h"

#define DAVE_MACRD_HEAD_MAX (4096)
#define DAVE_MCARD_CONTENT_MAX (16 * 1024 - DAVE_MACRD_HEAD_MAX)

typedef enum {
	MCardVer_reserve,
	MCardVer_text,
	MCardVer_media,
	MCardVer_talk,
	MCardVer_comment,
	MCardVer_max = 0x1fffffffffffffff
} MCardVer;

typedef enum {
	MCardType_Private_Message_Board,
	MCardType_Public_Message_Board,
	MCardType_POI,
	MCardType_TALK,
	MCardType_weichat_talk,
	MCardType_h5_talk,
	MCardType_max = 0x1fffffffffffffff
} MCardType;

typedef enum {
	MCardSource_aia,
	MCardSource_google,
	MCardSource_aws,
	MCardSource_aic,
	MCardSource_aib,
	MCardSource_aib_io,
	MCardSource_bbs,
	MCardSource_max = 0x1fffffffffffffff
} MCardSource;

typedef enum {
    MCardTimeType_Permanent,
    MCardTimeType_Normal,
    MCardTimeType_max = 0x1fffffffffffffff
} MCardTimeType;

typedef enum {
	MCardIdentityType_none,
	MCardIdentityType_initialization,
	MCardIdentityType_user,
	MCardIdentityType_AI,
	MCardIdentityType_max,
	MCardIdentityType_MAX = 0x1fffffffffffffff
} MCardIdentityType;

typedef enum {
	MCardContentType_begin,
	MCardContentType_utf8,
	MCardContentType_json,
	MCardContentType_xml,
	MCardContentType_login,
	MCardContentType_jpg_cropped_bin,
	MCardContentType_jpg_uncropped_bin,
	MCardContentType_image_sha1_id,
	MCardContentType_face_matching,
	MCardContentType_poster_generate,
	MCardContentType_start_up,
	MCardContentType_museum_page,
	MCardContentType_painter_page,
	MCardContentType_bbs_comment,
	MCardContentType_bbs_load,
	MCardContentType_painting_recommend,
	MCardContentType_portrait_transfer_list,
	MCardContentType_portrait_transfer_action,
	MCardContentType_landscape_transfer_list,
	MCardContentType_landscape_transfer_action,
	MCardContentType_image_search_cropped_bin,
	MCardContentType_image_search_uncropped_bin,
	MCardContentType_sculptures_search_cropped_bin,
	MCardContentType_sculptures_search_uncropped_bin,
	MCardContentType_menu_recognition,
	MCardContentType_tourism_image_search,
	MCardContentType_bagword,
	MCardContentType_museum_recommend,
	MCardContentType_painting_recommend_page,
	MCardContentType_museum_recommend_page,
	MCardContentType_museum_sha1_id,
	MCardContentType_refresh_recommend_cache,
	MCardContentType_detection,
	MCardContentType_generator,
	MCardContentType_end,
	MCardContentType_max = 0x1fffffffffffffff
} MCardContentType;

typedef struct {
    DateStruct write_time;
    MCardTimeType failure_type;
    DateStruct failure_time;
} MCardTime;

typedef struct {
    double latitude;
    double longitude;
    double altitude;
    double course;
	double slope;
} MCardLocation;

typedef struct {
	MCardLocation location;
	AIPlaceType type;
	s8 name[DAVE_POI_NAME_MAX];
	s8 phone_number[DAVE_MSISDN_LEN];
	s8 address[DAVE_POI_ADDRESS_MAX];
	double rating;
} MCardPOI;

typedef struct {
	MCardIdentityType id;
	MCardContentType content_type;
	LanguageCode content_language;
	MBUF *pContent;
} MCardContent;

typedef struct {
	MCardType type;
	s8 post_id[DAVE_KEY_LEN_MAX];
	s8 reply_comment_id[DAVE_KEY_LEN_MAX];
	s8 src_uuid[DAVE_UUID_LEN];
	s8 dst_uuid[DAVE_UUID_LEN];
	s8 src_user[DAVE_USER_NAME_LEN];
	s8 dst_user[DAVE_USER_NAME_LEN];
	MCardLocation location;
	MCardTime time;
	TerminalInformation ti;
	UserInformation user;
	ub comment_attributes;
} MCardCommentHeadData;

typedef struct {
	NoSQLHead nosql_head;
	MCardCommentHeadData comment_head;
	s8 reserve_data[DAVE_MACRD_HEAD_MAX - 2976];
} MCardCommentHead;

typedef struct {
	MCardVer version;
	MCardType type;
	s8 user[DAVE_USER_NAME_LEN];
	MCardLocation location;
	MCardTime time;
	MBUF *utf8_txt;
} MCardVerText;

typedef struct {
	MCardVer version;
	MCardType type;
	MCardSource source;
	MCardPOI poi;
	s8 author_name[DAVE_USER_NAME_LEN];
	s8 author_url[DAVE_URL_LEN];
	LanguageCode language;
	s8 author_photo_url[DAVE_URL_LEN];
	double rating;
	MBUF *utf8_txt;
	ub second;
} MCardVerMedia;

typedef struct {
	MCardVer version;
	MCardType type;
	s8 channel[DAVE_NORMAL_NAME_LEN];
	s8 uuid[DAVE_UUID_LEN];
	ub app_id;
	s8 src_user[DAVE_USER_NAME_LEN];
	s8 dst_user[DAVE_USER_NAME_LEN];
	MCardLocation location;
	MCardTime time;
	ub main_serial;
	ub sub_serial;
	ub total_sub_serial;
	MCardContent content;
} MCardVerTalk;

typedef struct {
	MCardVer version;
	MCardCommentHead head;
	MCardContent content;
} MCardVerComment;

typedef struct {
	MCardVer version;
	MCardVerText text;
	MCardVerMedia media;
	MCardVerTalk talk;
	MCardVerComment comment;
} MCard;

typedef struct {
	ub total_number;
	MCard mcard;
	void *next;
} MCardList;

#endif


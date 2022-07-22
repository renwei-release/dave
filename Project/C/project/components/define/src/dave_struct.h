/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_STRUCT_H__
#define __DAVE_STRUCT_H__

typedef struct {
	GPSBaseLocation longitude_base;
	ub ddd_longitude;
	ub dot_longitude;
	GPSBaseLocation latitude_base;
	ub ddd_latitude;
	ub dot_latitude;
} GPSInfo;

typedef struct {
    double latitude;
    double longitude;
    double altitude;
    double course;
} GPSLocation;

typedef struct {
	s8 token[DAVE_TOKEN_LEN];
	s8 key[DAVE_TOKEN_KEY_LEN];
} TokenVerification;

typedef struct {
	s8 label_extra_information[DAVE_LABEL_EXTRA_INFO_MAX];
	ub label_id;
	s8 label_str[DAVE_LABEL_STR_MAX];
	float label_score;
} UniversalLabel;

typedef struct {
	ub table_id;
	s8 painter_name[128];
	s8 painter_birth[128];
	s8 painter_death[128];
	s8 painter_introduction[16384];

	s8 painter_avatar_id[DAVE_SHA1_IMAGE_ID];
	s8 painter_avatar_url[512];
	s8 painter_related_painting[DAVE_KEY_OPT_MAX][DAVE_SHA1_IMAGE_ID];
	ub total_painting_number;
} PainterIntroduction;

typedef struct {
	ub table_id;
	s8 museum_id[DAVE_SHA1_IMAGE_ID];
	s8 museum_name[128];
	s8 address[1024];
	s8 ticket[256];
	s8 phone[128];
	s8 web_url[256];
	s8 opening_hours[256];
	s8 museum_introduction[16384];
} MuseumIntroduction;

typedef struct {
	s8 image_id[DAVE_SHA1_IMAGE_ID];

	PainterIntroduction painter;
	MuseumIntroduction museum;

	s8 creat_time[128];
	s8 dimensions[128];
	s8 title[DAVE_IMAGE_TITLE_LEN];
	s8 medium[128];
	s8 image_introduction[16384];
	s8 image_url[DAVE_URL_LEN];
	s8 page_url[DAVE_URL_LEN];
	s8 collection_location[512];
	s8 audio_url[DAVE_URL_LEN];
} ImageIntroduction;

typedef struct {
	s8 domain[DAVE_DOMAIN_LEN];
	s8 tenant_name[DAVE_TENANT_NAME_LEN];
	ub tenant_id;
} ChannelInfo;

typedef struct {
	s8 language[64];
	s8 country[64];
	s8 province[64];
	s8 city[64];
	s8 nickname[64];
	s8 gender[64];
	s8 brand[64];
	s8 model[64];
	s8 version[64];
	s8 system[64];
	s8 platform[64];
	s8 sdkversion[64];
} TerminalInformation;

typedef struct {
	s8 language[64];
	s8 country[64];
	s8 province[64];
	s8 city[64];
	s8 nickname[64];
	s8 gender[16];
	s8 phone_number_1[64];
	s8 phone_number_2[64];
	s8 email_1[64];
	s8 email_2[64];
	s8 home_address[128];
	s8 avatar_url[256];
} UserInformation;

typedef struct {
	s8 uuid[64];
	s8 openid[64];
	s8 code[64];
	s8 user_id[64];
	s8 session_key[64];
	s8 share_uuid[64];
	TerminalInformation ti;
} WeiChatUserInfo;

typedef struct {
	s8 key_str[DAVE_NOSQL_KEY_STR_MAX];
	s8 reserve_data[512 - DAVE_NOSQL_KEY_STR_MAX];
} NoSQLHead;

#endif


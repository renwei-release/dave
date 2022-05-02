/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __HTTP_PARAM_H__
#define __HTTP_PARAM_H__

#define DAVE_PATH_LEN (256)
#define DAVE_HTTP_KEY_LEN (128)
#define DAVE_HTTP_VALUE_LEN (512)
#define DAVE_HTTP_HEAD_MAX (16)

typedef enum{
	ListenHttp,
	ListenHttps,
	ListenWeb,
	ListenMax
} HTTPListenType;

typedef enum {
	LocationMatch_Accurate=0,
	LocationMatch_Prefix,
	LocationMatch_CaseRegular,
	LocationMatch_Regular,
	LocationMatch_CaseRegularExcl,
	LocationMatch_RegularExcl,
	LocationMatch_Wildcard,
	LocationMatch_Max
} HTTPMathcRule;

typedef enum {
	HttpMethod_post,
	HttpMethod_get,
	HttpMethod_put,
	HttpMethod_options,
	HttpMethod_delete,
	HttpMethod_max
} HttpMethod;

typedef enum {
	HttpContentType_json,
	HttpContentType_text,
	HttpContentType_xml,
	HttpContentType_xwww,
	HttpContentType_max
} HttpContentType;

typedef struct {
	s8 key[DAVE_HTTP_KEY_LEN];
	s8 value[DAVE_HTTP_VALUE_LEN];
} HttpKeyValue;

#endif


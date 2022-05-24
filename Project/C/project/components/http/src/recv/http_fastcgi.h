/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __HTTP_FASTCGI_H__
#define __HTTP_FASTCGI_H__

typedef enum {
	FCGI_BEGIN_REQUEST     = 1,
	FCGI_ABORT_REQUEST     = 2,
	FCGI_END_REQUEST       = 3,
	FCGI_PARAMS            = 4,
	FCGI_STDIN             = 5,
	FCGI_STDOUT            = 6,
	FCGI_STDERR            = 7,
	FCGI_DATA              = 8,
	FCGI_GET_VALUES        = 9,
	FCGI_GET_VALUES_RESULT = 10,
	FCGI_UNKNOWN_TYPE      = 11
} FcgiType;

typedef enum {
	FCGI_RESPONDER  = 1,
	FCGI_AUTHORIZER = 2,
	FCGI_FILTER     = 3
} FCGIRole;

typedef enum {
	FCGI_DONOT_KEEP_CONN = 0,
	FCGI_KEEP_CONN = 1,
} FCGIFlags;

typedef enum {
	FCGI_REQUEST_COMPLETE = 0,
	FCGI_CANT_MPX_CONN = 1,
	FCGI_OVERLOADED = 2,
	FCGI_UNKNOWN_ROLE = 3,
} FCGIProtocolStatus;

typedef struct {
	ub version;
	FcgiType type;
	ub request_id;
	ub content_length;
	ub padding_length;
	ub reserved;
} FcgiHead;

typedef struct {
	FCGIRole role;
 	FCGIFlags flags;
	u8 reserved[5];
} FCGIBeginRequestBody;

typedef struct {
     ub app_status;
     FCGIProtocolStatus protocol_status;
     u8 reserved[3];
} FCGIEndRequestBody;

typedef struct {
	s8 *name;
	s8 *value;
	void *next;
} FCGIParamsBody;

typedef struct {
	FcgiHead head;

	FCGIBeginRequestBody request;
	FCGIParamsBody *params;
	ub content_length_max;
	ub content_length;
	s8 *content;
} Fcgi;

dave_bool http_fastcgi_parse(u8 *data, ub data_len, Fcgi *pFcgi);

void http_fastcgi_parse_release(Fcgi *pFcgi);

ub http_fastcgi_build(Fcgi *pFcgi, HttpContentType content_type, s8 *content_ptr, ub content_len, u8 *cgi_ptr, ub cgi_len);

ub http_fastcgi_build_error(Fcgi *pFcgi, u8 *cgi_buf, ub cgi_len);

ub http_fastcgi_build_ok(Fcgi *pFcgi, u8 *cgi_buf, ub cgi_len);

ub http_fastcgi_load_head(HTTPRecvReq *pReq, FCGIParamsBody *pParams);

FCGIParamsBody * http_fastcgi_find_head(FCGIParamsBody *pParams, char *name);

#endif


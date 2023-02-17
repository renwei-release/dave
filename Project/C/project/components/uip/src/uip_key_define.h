/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UIP_KEY_DEFINE_H__
#define __UIP_KEY_DEFINE_H__
#include "dave_base.h"
#include "uip_msg.h"

#define UIP_VERSION 2

#define UIP_JSON_VERSION "uip_version"
#define UIP_JSON_HEAD "uip_head"
#define UIP_JSON_BODY "uip_body"

#define UIP_JSON_METHOD "METHOD"
#define UIP_JSON_CHANNEL "CHANNEL"
#define UIP_JSON_AUTH_KEY "AUTH_KEY"
#define UIP_JSON_CURRENT_MILLISECONDS "CURRENT_MILLISECONDS"
#define UIP_JSON_SERIAL "SERIAL"
#define UIP_JSON_CUSTOMER_HEAD "CUSTOMER_HEAD"
#define UIP_JSON_CUSTOMER_BODY "CUSTOMER_BODY"
#define UIP_JSON_RESULT_CODE "RESULT_CODE"
#define UIP_JSON_RESULT_DESC "RESULT_DESC"
#define UIP_JSON_BODY_DATA "__BODY_DATA__"

typedef struct {
	dave_bool req_flag;
	RetCode rsp_code;
	s8 method[DAVE_UIP_METHOD_MAX_LEN];
	s8 channel[DAVE_NORMAL_NAME_LEN];
	s8 auth_key_str[DAVE_AUTH_KEY_STR_LEN];
	ub current_milliseconds;
	ub serial;
	MBUF *customer_head;
} UIPHead;

typedef struct {
	void *pJson;
	MBUF *customer_body;
} UIPBody;

typedef struct {
	ub magic_data;

	ub version;
	UIPHead head;
	UIPBody body;

	ThreadId src;
	void *ptr;

	s8 remote_address[DAVE_URL_LEN];
	ub remote_port;
	UIPType uip_type;

	void *auto_release_json;

	s8 register_thread[DAVE_THREAD_NAME_LEN];
} UIPStack;

#endif


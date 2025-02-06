/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RTC_PARAM_H__
#define __RTC_PARAM_H__
#include "dave_base.h"

#define TLV_BUFFER_LENGTH_MAX 16 * 1024
#define TOKEN_DATA_BUFFER_MAX 4 * 1024
#define TOKEN_LIFT_MAX 16

typedef enum {
	RTCClientType_socket,
	RTCClientType_websocket,
} RTCClientType;

#endif


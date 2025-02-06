/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RTC_WEBSOCKET_H__
#define __RTC_WEBSOCKET_H__
#include "dave_base.h"
#include "rtc_main.h"

void rtc_websocket_init(void);
void rtc_websocket_exit(void);

void rtc_websocket_send(RTCClient *pClient, MBUF *data);

#endif


/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RTC_SOCKET_H__
#define __RTC_SOCKET_H__
#include "dave_base.h"
#include "rtc_main.h"

void rtc_socket_init(void);
void rtc_socket_exit(void);

void rtc_socket_send(RTCClient *pClient, MBUF *data);

#endif

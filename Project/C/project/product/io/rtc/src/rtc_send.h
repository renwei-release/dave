/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RTC_SEND_H__
#define __RTC_SEND_H__
#include "dave_base.h"

void rtc_send(RTCClient *pClient, s8 *tlv_ptr, ub tlv_len);

#endif


/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RTC_TOKEN_H__
#define __RTC_TOKEN_H__
#include "dave_base.h"

void rtc_token_init(void);
void rtc_token_exit(void);

s8 * rtc_token_creat(s8 *id, ThreadId src_id, s8 *src_gid, s8 *src_name);
RTCToken * rtc_token_inq(s8 *token);
void rtc_token_del(s8 *token);

void rtc_token_add_client(s8 *token, RTCClient *pClient);
RTCClient * rtc_token_inq_client(s8 *token);

#endif
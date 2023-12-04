/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RAMKV_TIMER_H__
#define __RAMKV_TIMER_H__

void ramkv_timer_init(KV *pKV, ub out_second, ramkv_time_callback outback_fun);

void ramkv_timer_exit(KV *pKV, s8 *fun, ub line);

void ramkv_timer_add(KV *pKV, u8 *key_ptr, ub key_len);

void ramkv_timer_inq(KV *pKV, u8 *key_ptr, ub key_len);

void ramkv_timer_del(KV *pKV, u8 *key_ptr, ub key_len, s8 *fun, ub line);

void ramkv_timer_out(KV *pKV);

ub ramkv_timer_info(KV *pKV, s8 *info_ptr, ub info_len);

#endif


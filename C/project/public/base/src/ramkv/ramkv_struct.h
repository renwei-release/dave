/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RAMKV_STRUCT_H__
#define __RAMKV_STRUCT_H__

void ramkv_struct_init(void);

void ramkv_struct_exit(void);

KV * __ramkv_malloc__(dave_bool external_call, s8 *name, KvAttrib attrib, ub out_second, ramkv_time_callback callback_fun, s8 *fun, ub line);
#define ramkv_malloc(external_call, name, attrib, out_second, callback_fun) __ramkv_malloc__(external_call, name, attrib, out_second, callback_fun, (s8 *)__func__, (ub)__LINE__)

void ramkv_free(dave_bool external_call, KV *pKV);

void ramkv_timer(TIMERID timer_id, ub thread_index, void *param);

dave_bool __ramkv_check__(KV *pKV, s8 *fun, ub line);
#define ramkv_check(pKV) __ramkv_check__(pKV, (s8 *)__func__, (ub)__LINE__)

#endif


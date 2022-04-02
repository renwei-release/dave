/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __KV_STRUCT_H__
#define __KV_STRUCT_H__

void kv_struct_init(void);

void kv_struct_exit(void);

KV * __kv_malloc__(dave_bool external_call, s8 *name, KVAttrib attrib, ub out_second, kv_out_callback callback_fun, s8 *fun, ub line);
#define kv_malloc(external_call, name, attrib, out_second, callback_fun) __kv_malloc__(external_call, name, attrib, out_second, callback_fun, (s8 *)__func__, (ub)__LINE__)

void kv_free(dave_bool external_call, KV *pKV);

void kv_timer(TIMERID timer_id, ub thread_index, void *param);

dave_bool __kv_check__(KV *pKV, s8 *fun, ub line);
#define kv_check(pKV) __kv_check__(pKV, (s8 *)__func__, (ub)__LINE__)

#endif


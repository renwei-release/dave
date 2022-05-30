/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_KV_H__
#define __BASE_KV_H__

typedef enum {
	KvAttrib_ram,		/* A memory KV, if there are not many storage nodes, you can use it, the fastest, but it takes up a lot of memory */
	KvAttrib_list,		/* A memory KV, if there are many storage nodes, you can use it, the speed is slower than ram, and it takes up less memory */
	KvAttrib_remote,	/* A remote KV can realize clustering */
} KvAttrib;

typedef void (* ramkv_time_callback)(void *ramkv, s8 *key);
typedef RetCode (* ramkv_recycle_callback)(void *ramkv, s8 *key);

void base_ramkv_init(void);
void base_ramkv_exit(void);

dave_bool base_ramkv_check(void *ramkv);
ub base_ramkv_info(void *ramkv, s8 *info_ptr, ub info_len);
void base_ramkv_test(s8 *cmd);

void * __base_ramkv_malloc__(dave_bool external_call, s8 *name, KvAttrib attrib, ub out_second, ramkv_time_callback callback_fun, s8 *fun, ub line);
void __base_ramkv_free__(dave_bool external_call, void *ramkv, ramkv_recycle_callback callback_fun, s8 *fun, ub line);

dave_bool __base_ramkv_add_key_ptr__(void *ramkv, s8 *key, void *ptr, s8 *fun, ub line);
void * __base_ramkv_inq_key_ptr__(void *ramkv, s8 *key, s8 *fun, ub line);
void * __base_ramkv_inq_index_ptr__(void *ramkv, ub index, s8 *fun, ub line);
void * __base_ramkv_del_key_ptr__(void *ramkv, s8 *key, s8 *fun, ub line);
dave_bool __base_ramkv_add_ub_ptr__(void *ramkv, ub ub_key, void *ptr, s8 *fun, ub line);
void * __base_ramkv_inq_ub_ptr__(void *ramkv, ub ub_key, s8 *fun, ub line);
void * __base_ramkv_del_ub_ptr__(void *ramkv, ub ub_key, s8 *fun, ub line);
dave_bool __base_ramkv_add_bin_ptr__(void *ramkv, u8 *bin_data, ub bin_len, void *ptr, s8 *fun, ub line);
void * __base_ramkv_inq_bin_ptr__(void *ramkv, u8 *bin_data, ub bin_len, s8 *fun, ub line);
void * __base_ramkv_del_bin_ptr__(void *ramkv, u8 *bin_data, ub bin_len, s8 *fun, ub line);
void * __base_ramkv_inq_top_ptr__(void *ramkv, s8 *fun, ub line);

#define base_ramkv_malloc(name, attrib, out_second, callback_fun) __base_ramkv_malloc__(dave_true, name, attrib, out_second, callback_fun, (s8 *)__func__, (ub)__LINE__)
#define base_ramkv_free(ramkv, callback_fun) __base_ramkv_free__(dave_true, ramkv, callback_fun, (s8 *)__func__, (ub)__LINE__)
#define base_ramkv_add_key_ptr(ramkv, key, ptr) __base_ramkv_add_key_ptr__(ramkv, key, ptr, (s8 *)__func__, (ub)__LINE__)
#define base_ramkv_inq_key_ptr(ramkv, key) __base_ramkv_inq_key_ptr__(ramkv, key, (s8 *)__func__, (ub)__LINE__)
#define base_ramkv_inq_index_ptr(ramkv, index) __base_ramkv_inq_index_ptr__(ramkv, index, (s8 *)__func__, (ub)__LINE__)
#define base_ramkv_del_key_ptr(ramkv, key) __base_ramkv_del_key_ptr__(ramkv, key, (s8 *)__func__, (ub)__LINE__)
#define base_ramkv_add_ub_ptr(ramkv, ub_key, ptr) __base_ramkv_add_ub_ptr__(ramkv, ub_key, ptr, (s8 *)__func__, (ub)__LINE__)
#define base_ramkv_inq_ub_ptr(ramkv, ub_key) __base_ramkv_inq_ub_ptr__(ramkv, ub_key, (s8 *)__func__, (ub)__LINE__)
#define base_ramkv_del_ub_ptr(ramkv, ub_key) __base_ramkv_del_ub_ptr__(ramkv, ub_key, (s8 *)__func__, (ub)__LINE__)
#define base_ramkv_add_bin_ptr(ramkv, bin_data, bin_len, ptr) __base_ramkv_add_bin_ptr__(ramkv, bin_data, bin_len, ptr, (s8 *)__func__, (ub)__LINE__)
#define base_ramkv_inq_bin_ptr(ramkv, bin_data, bin_len) __base_ramkv_inq_bin_ptr__(ramkv, bin_data, bin_len, (s8 *)__func__, (ub)__LINE__)
#define base_ramkv_del_bin_ptr(ramkv, bin_data, bin_len) __base_ramkv_del_bin_ptr__(ramkv, bin_data, bin_len, (s8 *)__func__, (ub)__LINE__)
#define base_ramkv_inq_top_ptr(ramkv) __base_ramkv_inq_top_ptr__(ramkv, (s8 *)__func__, (ub)__LINE__)

#define kv_malloc base_ramkv_malloc
#define kv_free base_ramkv_free
#define kv_add_key_ptr base_ramkv_add_key_ptr
#define kv_inq_key_ptr base_ramkv_inq_key_ptr
#define kv_del_key_ptr base_ramkv_del_key_ptr
#define kv_add_ub_ptr base_ramkv_add_ub_ptr
#define kv_inq_ub_ptr base_ramkv_inq_ub_ptr
#define kv_del_ub_ptr base_ramkv_del_ub_ptr

#endif


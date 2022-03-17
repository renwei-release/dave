/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_KV_H__
#define __BASE_KV_H__

typedef enum {
	KVAttrib_ram,		/* A memory KV, if there are not many storage nodes, you can use it, the fastest, but it takes up a lot of memory */
	KVAttrib_list,		/* A memory KV, if there are many storage nodes, you can use it, the speed is slower than ram, and it takes up less memory */
	KVAttrib_remote,	/* A remote KV can realize clustering */
} KVAttrib;

typedef void (* kv_out_callback)(void *kv, s8 *key);

typedef ErrCode (* kv_free_recycle_callback)(void *kv, s8 *key);

void base_kv_init(void);
void base_kv_exit(void);

void * __base_kv_malloc__(dave_bool external_call, s8 *name, KVAttrib attrib, ub out_second, kv_out_callback callback_fun, s8 *fun, ub line);
void __base_kv_free__(dave_bool external_call, void *kv, kv_free_recycle_callback callback_fun, s8 *fun, ub line);

dave_bool base_kv_check(void *kv);

dave_bool __base_kv_add_key_ptr__(void *kv, s8 *key, void *ptr, s8 *fun, ub line);
void * __base_kv_inq_key_ptr__(void *kv, s8 *key, s8 *fun, ub line);
void * __base_kv_inq_index_ptr__(void *kv, ub index, s8 *fun, ub line);
void * __base_kv_del_key_ptr__(void *kv, s8 *key, s8 *fun, ub line);
dave_bool __base_kv_add_ub_ptr__(void *kv, ub ub_key, void *ptr, s8 *fun, ub line);
void * __base_kv_inq_ub_ptr__(void *kv, ub ub_key, s8 *fun, ub line);
void * __base_kv_del_ub_ptr__(void *kv, ub ub_key, s8 *fun, ub line);
dave_bool __base_kv_add_bin_ptr__(void *kv, u8 *bin_data, ub bin_len, void *ptr, s8 *fun, ub line);
void * __base_kv_inq_bin_ptr__(void *kv, u8 *bin_data, ub bin_len, s8 *fun, ub line);
void * __base_kv_del_bin_ptr__(void *kv, u8 *bin_data, ub bin_len, s8 *fun, ub line);
void * __base_kv_inq_top_ptr__(void *kv, s8 *fun, ub line);

#define base_kv_malloc(name, attrib, out_second, callback_fun) __base_kv_malloc__(dave_true, name, attrib, out_second, callback_fun, (s8 *)__func__, (ub)__LINE__)
#define base_kv_free(kv, callback_fun) __base_kv_free__(dave_true, kv, callback_fun, (s8 *)__func__, (ub)__LINE__)
#define base_kv_add_key_ptr(kv, key, ptr) __base_kv_add_key_ptr__(kv, key, ptr, (s8 *)__func__, (ub)__LINE__)
#define base_kv_inq_key_ptr(kv, key) __base_kv_inq_key_ptr__(kv, key, (s8 *)__func__, (ub)__LINE__)
#define base_kv_inq_index_ptr(kv, index) __base_kv_inq_index_ptr__(kv, index, (s8 *)__func__, (ub)__LINE__)
#define base_kv_del_key_ptr(kv, key) __base_kv_del_key_ptr__(kv, key, (s8 *)__func__, (ub)__LINE__)
#define base_kv_add_ub_ptr(kv, ub_key, ptr) __base_kv_add_ub_ptr__(kv, ub_key, ptr, (s8 *)__func__, (ub)__LINE__)
#define base_kv_inq_ub_ptr(kv, ub_key) __base_kv_inq_ub_ptr__(kv, ub_key, (s8 *)__func__, (ub)__LINE__)
#define base_kv_del_ub_ptr(kv, ub_key) __base_kv_del_ub_ptr__(kv, ub_key, (s8 *)__func__, (ub)__LINE__)
#define base_kv_add_bin_ptr(kv, bin_data, bin_len, ptr) __base_kv_add_bin_ptr__(kv, bin_data, bin_len, ptr, (s8 *)__func__, (ub)__LINE__)
#define base_kv_inq_bin_ptr(kv, bin_data, bin_len) __base_kv_inq_bin_ptr__(kv, bin_data, bin_len, (s8 *)__func__, (ub)__LINE__)
#define base_kv_del_bin_ptr(kv, bin_data, bin_len) __base_kv_del_bin_ptr__(kv, bin_data, bin_len, (s8 *)__func__, (ub)__LINE__)
#define base_kv_inq_top_ptr(kv) __base_kv_inq_top_ptr__(kv, (s8 *)__func__, (ub)__LINE__)

#endif


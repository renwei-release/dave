/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RAMKV_PARAM_H__
#define __RAMKV_PARAM_H__
#include "dave_3rdparty.h"

#define KV_NAME_MAX 64
#define KV_KEY_MAX 512

#ifdef LEVEL_PRODUCT_alpha
#define ramkvm_malloc dave_malloc
#define ramkvm_malloc_line(len, fun, line) __base_malloc__(len, dave_false, 0x00, fun, line)
#define ramkvm_free dave_free
#elif defined(PERFTOOLS_3RDPARTY)
#define ramkvm_malloc dave_perftools_malloc
#define ramkvm_malloc_line(len, fun, line) dave_perftools_malloc(len)
#define ramkvm_free dave_perftools_free
#elif defined(JEMALLOC_3RDPARTY)
#define ramkvm_malloc dave_jemalloc
#define ramkvm_malloc_line(len, fun, line) dave_jemalloc(len)
#define ramkvm_free dave_jefree
#else
#define ramkvm_malloc dave_os_malloc
#define ramkvm_free dave_os_free
#endif

#include "ramkv_list_param.h"
#include "ramkv_remote_param.h"

typedef struct {
	void *pMultiMap;
	KVList ramkv_list;
} KVLocal;

typedef struct {
	ub key_len;
	u8 key_data[KV_KEY_MAX];

	void *pTimerLine;

	void *up;
	void *next;
} KVTimerKeyList;

typedef struct {
	sb current_times;
	KVTimerKeyList *pKeyHead;
	KVTimerKeyList *pKeyTail;
	void *next;
} KVTimerLineList;

typedef struct {
	ub out_second;
	ub base_timer;
	ub out_times;
	dave_bool inq_update_timer;
	ramkv_time_callback callback_fun;
	TIMERID timer_id;
	KVTimerLineList *timer_line;
	void *key_ramkv;
} KVTimer;

typedef struct {
	ub magic_data;
	ub magic_rand;

	TLock ramkv_pv;

	s8 thread_name[DAVE_THREAD_NAME_LEN];

	KVTimer ramkv_timer;

	s8 name[KV_NAME_MAX];
	KvAttrib attrib;

	KVLocal local;
	KVRemote remote;
} KV;

#endif


/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __KV_PARAM_H__
#define __KV_PARAM_H__
#include "dave_third_party.h"

#define KV_NAME_MAX 64
#define KV_KEY_MAX 512

#ifdef LEVEL_PRODUCT_alpha
#define kvm_malloc dave_malloc
#define kvm_malloc_line(len, fun, line) __base_malloc__(len, dave_false, 0x00, fun, line)
#define kvm_free dave_free
#elif defined(PERFTOOLS_3RDPARTY)
#define kvm_malloc dave_perftools_malloc
#define kvm_malloc_line(len, fun, line) dave_perftools_malloc(len)
#define kvm_free dave_perftools_free
#elif defined(JEMALLOC_3RDPARTY)
#define kvm_malloc dave_jemalloc
#define kvm_malloc_line(len, fun, line) dave_jemalloc(len)
#define kvm_free dave_jefree
#else
#define kvm_malloc dave_os_malloc
#define kvm_free dave_os_free
#endif

#include "kv_list_param.h"
#include "kv_remote_param.h"

typedef struct {
	void *pMultiMap;
	KVList kv_list;
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
	kv_out_callback callback_fun;
	TIMERID timer_id;
	KVTimerLineList *timer_line;
	void *key_kv;
} KVTimer;

typedef struct {
	ub magic_data;
	ub magic_rand;

	DaveLock kv_pv;

	s8 thread_name[DAVE_THREAD_NAME_LEN];

	KVTimer kv_timer;

	s8 name[KV_NAME_MAX];
	KVAttrib attrib;

	KVLocal local;
	KVRemote remote;
} KV;

#endif


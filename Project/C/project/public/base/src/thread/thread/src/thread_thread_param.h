/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_THREAD_PARAM_H__
#define __THREAD_THREAD_PARAM_H__
#include "base_macro.h"
#include "thread_struct.h"

#define ThreadState_INIT 0
#define	ThreadState_RUNNING 1
#define	ThreadState_SLEEP 2
#define	ThreadState_STOPPED 3
#define	ThreadState_MAX 4

typedef struct {
	s8 thread_name[THREAD_NAME_MAX];
	ub thread_index;
	ThreadId thread_id;
	ThreadQueue thread_queue[THREAD_THREAD_QUEUE_NUM];
	ThreadChain chain;
	ThreadRouter router;

	volatile ub state;

	pthread_t thr_id;
	pthread_mutex_t m_mutex_t;
	pthread_cond_t  m_cond_t;

	ub wakeup_index;

	ThreadSync sync;
	void *current_coroutine_point;
} ThreadThread;

typedef struct {
	ThreadThread *pTThread;
	void *next;
} __ThreadThreadList__;

typedef struct {
	ub thread_index;
	s8 thread_name[DAVE_THREAD_NAME_LEN];

	__ThreadThreadList__ *pList;

	__ThreadThreadList__ *pLoop;
} ThreadThreadList;

#endif


/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_STRUCT_H__
#define __THREAD_STRUCT_H__
#include "dave_tools.h"
#include "thread_parameter.h"

typedef struct {
	MSGBODY msg_body;

	void *pQueue;

	void *next;
} ThreadMsg;

typedef struct {
	TLock sync_pv;
	s8 wait_name[THREAD_NAME_MAX];
	ThreadId wait_thread;
	ub wait_msg;
	u8 *wait_body;
	ub wait_len;
	ub wait_time;
} ThreadSync;

typedef struct {
	TLock queue_opt_pv;

	dave_bool on_queue_process;

	ub list_number;
	ThreadMsg *queue_head;
	ThreadMsg *queue_tail;

	// queue debug message.
	ub queue_received_counter;
	ub queue_processed_counter;
} ThreadQueue;

typedef struct {
	ThreadId thread_id;
	s8 thread_name[THREAD_NAME_MAX];
	ub level_number;
	ub thread_flag;
	TaskAttribute attrib;
	dave_bool has_initialization;
	base_thread_fun thread_init;
	base_thread_fun thread_main;
	base_thread_fun thread_exit;

	// thread genealogy.
	ThreadId father;
	ThreadId child[THREAD_MAX];

	// check thread on idle logic.
	ub message_idle_time;
	ub message_idle_total;
	ub message_wakeup_counter;

	// thread message struct.
	ub msg_queue_write_sequence;
	ub msg_queue_read_sequence;
	ThreadQueue msg_queue[THREAD_MSG_QUEUE_NUM];
	ub seq_queue_read_sequence;
	ThreadQueue seq_queue[THREAD_SEQ_QUEUE_NUM];

	// sync action
	ThreadSync sync;

	dave_bool has_not_wakeup_flag;

	ub thread_index;
	
	dave_bool trace_on;
} ThreadStruct;

typedef struct {
	ub msg_id;
	thread_msg_fun msg_fun;
	void *user_ptr;
} MsgCallFun;

#endif


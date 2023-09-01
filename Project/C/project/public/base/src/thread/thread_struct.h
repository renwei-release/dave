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
#include "thread_chain.h"
#include "thread_router.h"

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

	ub msg_number;
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

	/*
	 * General message queues, thread preemptions and high concurrent,
	 * external interface messages are generally queue.
	 */
	ub msg_queue_write_sequence;
	ub msg_queue_read_sequence;
	ThreadQueue msg_queue[THREAD_MSG_QUEUE_NUM];

	/*
	 * Sequentially execute the message queue, enter the message of this queue,
	 * distinguish between business logic, no concurrent, is executed in order.
	 */
	ub seq_queue_read_sequence;
	ThreadQueue seq_queue[THREAD_SEQ_QUEUE_NUM];

	/*
	 * Priority to execute a message queue, the message that enters this
	 * queue will be executed fastest, and this message queue can be concurrent.
	 */
	ub pre_queue_write_sequence;
	ub pre_queue_read_sequence;
	ThreadQueue pre_queue[THREAD_PRE_QUEUE_NUM];

	/*
	 * Used to record the field execution of coroutines.
	 */
	ub coroutines_site_creat_counter;
	ub coroutines_site_release_counter;

	// sync action
	ThreadSync sync;

	// call chain
	ThreadChain chain;

	// call router
	ThreadRouter router;

	dave_bool has_not_wakeup_flag;

	ub thread_index;
	
	dave_bool trace_on;
} ThreadStruct;

typedef struct {
	ThreadId thread_id;
	ub msg_id;
	base_thread_fun msg_fun;
	void *user_ptr;
} MsgCallFun;

typedef struct {
	void *next_stack;
	ThreadId thread_id;
} ThreadStack;

#endif


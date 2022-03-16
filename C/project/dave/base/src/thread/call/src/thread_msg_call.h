/*
 * ================================================================================
 * (c) Copyright 2017 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2017.08.22.
 * ================================================================================
 */

#ifndef __THREAD_MSG_CALL_H__
#define __THREAD_MSG_CALL_H__
#include "base_define.h"

void thread_msg_call_init(void);

void thread_msg_call_exit(void);

dave_bool thread_msg_call_register(ThreadId thread_id, ub msg_id, thread_msg_fun msg_fun, void *user_ptr);

void thread_msg_call_unregister(ThreadId thread_id, ub msg_id);

MsgCallFun * thread_msg_call(ThreadId thread_id, ub msg_id);

#endif


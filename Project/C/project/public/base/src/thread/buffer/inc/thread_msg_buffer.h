/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_MSG_BUFFER_H__
#define __THREAD_MSG_BUFFER_H__

void thread_msg_buffer_init(void);

void thread_msg_buffer_exit(void);

dave_bool thread_msg_buffer_thread_push(
	ThreadId src_id, s8 *dst_thread,
	BaseMsgType msg_type,
	ub msg_id, ub msg_len, u8 *msg_body,
	s8 *fun, ub line);

void thread_msg_buffer_thread_pop(s8 *dst_thread);

dave_bool thread_msg_buffer_gid_push(
	ThreadId src_id, s8 *gid, s8 *dst_thread,
	BaseMsgType msg_type,
	ub msg_id, ub msg_len, u8 *msg_body,
	s8 *fun, ub line);

void thread_msg_buffer_gid_pop(s8 *gid, s8 *dst_thread);

dave_bool thread_msg_buffer_uid_push(
	ThreadId src_id, s8 *uid,
	BaseMsgType msg_type,
	ub msg_id, ub msg_len, u8 *msg_body,
	s8 *fun, ub line);

void thread_msg_buffer_uid_pop(s8 *uid);

#endif


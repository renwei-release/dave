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

dave_bool thread_msg_buffer(ThreadId src_id, s8 *dst_thread_name, BaseMsgType msg_type, ub msg_id, ub msg_len, u8 *msg_body, s8 *fun, ub line);

void thread_msg_buffer_action(s8 *thread_name);

void thread_msg_buffer_tick(void);

#endif


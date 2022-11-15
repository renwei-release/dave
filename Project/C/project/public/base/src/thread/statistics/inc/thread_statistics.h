/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_STATISTICS_H__
#define __THREAD_STATISTICS_H__

void thread_statistics_init(void);

void thread_statistics_exit(void);

ub thread_statistics_enable(s8 *msg_ptr, ub msg_len);

ub thread_statistics_disable(s8 *msg_ptr, ub msg_len);

ub thread_statistics_info(s8 *msg_ptr, ub msg_len);

ub thread_statistics_start_msg(MSGBODY *msg);

void thread_statistics_end_msg(ub run_time, ThreadStruct *pThread, MSGBODY *msg);

#endif


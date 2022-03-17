/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_STATISTICS_H__
#define __THREAD_STATISTICS_H__

ub thread_statistics_run_start(void);

void thread_statistics_run_end(ub run_time, ThreadStruct *pThread, MSGBODY *msg);

void thread_statistics_write_msg_time(ThreadMsg *pMsg);

void thread_statistics_read_msg_time(ThreadMsg *pMsg);

void thread_statistics_setup_run_time(ub run_time);

void thread_statistics_setup_msg_time(ub msg_time);

void thread_statistics_setup_msg_id(ub msg_id);

void thread_statistics_setup_all_time(ub all_time);

void thread_statistics_load_all_time(ub *run_time, ub *msg_time);

#endif


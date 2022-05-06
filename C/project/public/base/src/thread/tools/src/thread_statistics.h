/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_STATISTICS_H__
#define __THREAD_STATISTICS_H__

ub thread_statistics_start_msg(MSGBODY *msg);

void thread_statistics_end_msg(ub run_time, ThreadStruct *pThread, MSGBODY *msg);

void thread_statistics_write_msg(ThreadMsg *pMsg);

void thread_statistics_read_msg(ThreadMsg *pMsg);

void thread_statistics_setup_all(ub msg_id, ub run_time, ub wakeup_time);

void thread_statistics_load_all(ub *msg_id, ub *run_time, ub *msg_time);

#endif


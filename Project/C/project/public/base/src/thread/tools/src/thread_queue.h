/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_QUEUE_H__
#define __THREAD_QUEUE_H__

void thread_queue_booting(ThreadQueue *pQueue, ub queue_number);

void thread_queue_reset(ThreadQueue *pQueue, ub queue_number);

void thread_queue_malloc(ThreadQueue *pQueue, ub queue_number);

void thread_queue_free(ThreadQueue *pQueue, ub queue_number);

RetCode thread_queue_write(ThreadQueue *pQueue, ThreadMsg *pMsg);

ThreadMsg * thread_queue_read(ThreadQueue *pQueue, dave_bool seq_flag);

void thread_queue_reset_process(ThreadQueue *pQueue);

ub thread_queue_num_msg(ThreadQueue *pQueue_ptr, ub queue_number, ub msg_id);

void thread_queue_total(ub *list, ub *received, ub *processed, ThreadQueue *pQueue_ptr, ub queue_number);

ub thread_queue_list(ThreadQueue *pQueue_ptr, ub queue_number);

#endif

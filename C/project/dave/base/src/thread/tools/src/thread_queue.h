/*
 * ================================================================================
 * (c) Copyright 2021 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2021.11.03.
 * ================================================================================
 */

#ifndef __THREAD_QUEUE_H__
#define __THREAD_QUEUE_H__

void thread_queue_all_reset(ThreadStruct *pThread);

void thread_queue_all_free(ThreadStruct *pThread);

void thread_queue_all_malloc(ThreadStruct *pThread);

ErrCode thread_queue_write(ThreadQueue *pQueue, ThreadMsg *pMsg);

ThreadMsg * thread_queue_read(ThreadQueue *pQueue, dave_bool seq_flag);

void thread_queue_reset_process(ThreadQueue *pQueue);

ub thread_queue_num_msg(ThreadQueue *pQueue_ptr, ub queue_number, ub msg_id);

void thread_queue_total(ub *list, ub *received, ub *processed, ThreadQueue *pQueue_ptr, ub queue_number);

ub thread_queue_list(ThreadQueue *pQueue_ptr, ub queue_number);

#endif


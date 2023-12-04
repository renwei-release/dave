/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_QUEUE_OPT_H__
#define __THREAD_QUEUE_OPT_H__

void thread_queue_booting(ThreadQueue *pQueue, ub queue_number);

void thread_queue_reset(ThreadQueue *pQueue, ub queue_number);

void thread_queue_malloc(ThreadQueue *pQueue, ub queue_number);

void thread_queue_free(ThreadQueue *pQueue, ub queue_number);

ub thread_queue_write(ThreadQueue *pQueue, ThreadMsg *pMsg);

ThreadMsg * thread_queue_read(ThreadQueue *pQueue);

ThreadMsg * thread_queue_clone(ThreadQueue *pQueue);

ThreadMsg * thread_queue_on_process_up(ThreadQueue *pQueue);

void thread_queue_on_process_down(ThreadQueue *pQueue);

void thread_queue_total_detail(ub *unprocessed, ub *received, ub *processed, ThreadQueue *pQueue_ptr, ub queue_number);

static inline ub
thread_queue_total_number(ThreadQueue *pQueue_ptr, ub queue_number) {
	ub queue_index, list_total_counter;
	ThreadQueue *pQueue;
	
	list_total_counter = 0;
	
	for(queue_index=0; queue_index<queue_number; queue_index++)
	{
		pQueue = &(pQueue_ptr[queue_index]);
	
		list_total_counter += (pQueue->msg_number);
	}
	
	return list_total_counter;
}

ub thread_queue_total_msg(ThreadQueue *pQueue_ptr, ub queue_number, ub msg_id);

#endif


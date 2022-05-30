/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_SYNC_H__
#define __THREAD_SYNC_H__
#include "dave_base.h"

#if defined(__DAVE_PRODUCT_SYNC__) || defined(__DAVE_PRODUCT_BASE__)
#define SYNC_STACK_SERVER
#endif
#if !(defined(__DAVE_PRODUCT_SYNC__) || defined(__DAVE_PRODUCT_LOG__))
#define SYNC_STACK_CLIENT
#endif

void thread_sync_init(void);
void thread_sync_exit(void);
ThreadId thread_sync_thread_id(ThreadId thread_id);

#endif


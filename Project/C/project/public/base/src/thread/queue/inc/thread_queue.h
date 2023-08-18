/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_QUEUE_H__
#define __THREAD_QUEUE_H__
#include "dave_base.h"

#if defined(__DAVE_PRODUCT_QUEUE__)
#define QUEUE_STACK_SERVER
#endif
#if ! defined(QUEUE_STACK_SERVER)
#define QUEUE_STACK_CLIENT
#endif

void thread_queue_init(void);
void thread_queue_exit(void);

#endif


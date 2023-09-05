/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_SELF_MAP_H__
#define __THREAD_SELF_MAP_H__
#include "base_macro.h"
#include "thread_thread_param.h"

void thread_self_map_init(void);
void thread_self_map_exit(void);

void thread_self_map_add(pthread_t self, ThreadThread *pTThread);
ThreadThread * thread_self_map_inq(pthread_t self);
ThreadThread * thread_self_map_del(pthread_t self);

#endif


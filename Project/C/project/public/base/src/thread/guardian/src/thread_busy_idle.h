/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_BUSY_IDLE_H__
#define __THREAD_BUSY_IDLE_H__

void thread_busy_idle_init(ThreadStruct *thread_struct);

void thread_busy_idle_exit(void);

void thread_busy_idle_check(void);

void thread_busy_idle_app_busy(dave_bool cfg_flag);
void thread_busy_idle_app_idle(dave_bool cfg_flag);

#endif


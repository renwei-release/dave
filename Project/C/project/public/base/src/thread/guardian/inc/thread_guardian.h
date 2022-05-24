/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_GUARDIAN_H__
#define __THREAD_GUARDIAN_H__

ThreadId thread_guardian_init(ThreadStruct *thread_struct);

void thread_guardian_exit(void);

#endif


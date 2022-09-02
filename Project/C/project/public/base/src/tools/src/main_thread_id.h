/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __MAIN_THREAD_ID_H__
#define __MAIN_THREAD_ID_H__

ThreadId main_thread_id_get(void);

void main_thread_id_set(ThreadId main_thread);

#endif


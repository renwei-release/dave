/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_LOCK_H__
#define __THREAD_LOCK_H__

void thread_lock_init(void);

void thread_lock_exit(void);

void thread_lock(void);

void thread_unlock(void);

void thread_exter_lock(void);

void thread_exter_unlock(void);

#endif


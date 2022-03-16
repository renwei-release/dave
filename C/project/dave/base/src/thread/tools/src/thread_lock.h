/*
 * ================================================================================
 * (c) Copyright 2018 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2018.06.25.
 * ================================================================================
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


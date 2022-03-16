/*
 * ================================================================================
 * (c) Copyright 2020 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.10.10.
 * ================================================================================
 */

#ifndef __THREAD_BUSY_IDLE_H__
#define __THREAD_BUSY_IDLE_H__

void thread_busy_idle_init(ThreadStruct *thread_struct);

void thread_busy_idle_exit(void);

void thread_busy_idle_cfg_update(CFGUpdate *pUpdate);

void thread_busy_idle_check(void);

#endif


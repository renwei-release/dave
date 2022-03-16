/*
 * ================================================================================
 * (c) Copyright 2018 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2018.06.22.
 * ================================================================================
 */

#ifndef __THREAD_GUARDIAN_H__
#define __THREAD_GUARDIAN_H__

ThreadId thread_guardian_init(ThreadStruct *thread_struct);

void thread_guardian_exit(void);

#endif


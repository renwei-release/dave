/*
 * ================================================================================
 * (c) Copyright 2017 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2017.08.16.
 * ================================================================================
 */

#ifndef __THREAD_MAP_H__
#define __THREAD_MAP_H__

void thread_map_init(void);

void thread_map_exit(void);

dave_bool __thread_map_name_add__(s8 *thread_name, ThreadStruct *pThread, s8 *fun, ub line);
#define thread_map_name_add(thread_name, pThread) __thread_map_name_add__(thread_name, pThread, (s8 *)__func__, (ub)__LINE__)

void __thread_map_name_del__(s8 *thread_name, s8 *fun, ub line);
#define thread_map_name_del(thread_name) __thread_map_name_del__(thread_name, (s8 *)__func__, (ub)__LINE__)

ThreadStruct * __thread_map_name__(s8 *thread_name, s8 *fun, ub line);
#define thread_map_name(thread_name) __thread_map_name__(thread_name, (s8 *)__func__, (ub)__LINE__)

ThreadId __thread_map_id__(s8 *thread_name, s8 *fun, ub line);
#define thread_map_id(thread_name) __thread_map_id__(thread_name, (s8 *)__func__, (ub)__LINE__)

#endif


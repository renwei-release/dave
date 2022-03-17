/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_LOG_H__
#define __THREAD_LOG_H__
#include "dave_base.h"

#ifdef __BASE_ALPHA_VERSION__
// #define THREAD_DEBUG
 #define THREAD_MEM_DEBUG
// #define THREAD_MEM_TRACE
#endif

#ifdef THREAD_DEBUG
// #define DEBUG_THREAD_RUN_INFO
#endif

#ifdef THREAD_DEBUG
#define THREADDEBUG(a, ...) { DAVEDEBUG("[THREAD]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define THREADDEBUG(a, ...) {}
#endif

#ifdef THREAD_MEM_TRACE
#define THREADMEM(a, ...) { DAVETRACE("[THREAD Memory]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); }
#else
#define THREADMEM(a, ...) {}
#endif

#define THREADTRACE(a, ...) { TRACETENABLE { DAVETRACE("[THREAD]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\r\n"); } }

#define THREADLTRACE(t,n,a,...) { TRACELENABLE(t,n) { DAVELOG("[THREAD]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); } }

#define THREADLOG(a, ...) { DAVELOG("[THREAD]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#define THREADABNOR(a, ...) { DAVEABNORMAL("[THREAD Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#endif


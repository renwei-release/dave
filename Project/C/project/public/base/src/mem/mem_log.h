/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __MEM_LOG_H__
#define __MEM_LOG_H__
#include "base_macro.h"
#include "dave_base.h"

#if defined(LEVEL_PRODUCT_alpha)
 #define MEMORY_DEBUG
// #define MEMORY_TRACE
// #define MEMORY_TEST
#endif

#ifdef LEVEL_PRODUCT_alpha
// #define MEMORY_DEBUG_SHOW_INIT_INFO
#endif

#ifdef MEMORY_TRACE
#define MEMTRACE(a, ...) { DAVEDEBUG("[MEM]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define MEMTRACE(a, ...) {}
#endif

#ifdef MEMORY_DEBUG_SHOW_INIT_INFO
#define MEMINITTRACE(a, ...) { DAVEDEBUG("[MEM]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define MEMINITTRACE(a, ...) {}
#endif

#define MEMLTRACE(t,n,a,...) { TRACELENABLE(t,n) { DAVELOG("[MEM]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); } }

#define MEMLOG(a, ...) { DAVELOG("[MEM]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#define MEMABNOR(a, ...) { DAVEABNORMAL("[MEM Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#endif


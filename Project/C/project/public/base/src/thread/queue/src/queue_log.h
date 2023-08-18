/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __QUEUE_LOG_H__
#define __QUEUE_LOG_H__
#include "dave_base.h"

#if defined(LEVEL_PRODUCT_alpha)
// #define QUEUE_DEBUG
#endif

#ifdef QUEUE_DEBUG
#define QUEUEDEBUG(a, ...) { DAVETRACE("[QUEUE]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); }
#else
#define QUEUEDEBUG(a, ...)
#endif

#define QUEUETRACE(a, ...) { TRACETENABLE { DAVETRACE("[QUEUE]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define QUEUELTRACE(t,n,a,...) { TRACELENABLE(t,n) { DAVETRACE("[QUEUE]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define QUEUEABNOR(a, ...) { DAVEABNORMAL("[QUEUE Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define QUEUELOG(a, ...) { DAVELOG("[SYNC]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


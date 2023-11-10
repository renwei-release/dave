/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __LOG_LOG_H__
#define __LOG_LOG_H__
#include "base_log.h"

#ifdef LEVEL_PRODUCT_alpha
// #define LOG_DEBUG
#endif

#ifdef LOG_DEBUG
#define LOGDEBUG(a, ...) { DAVEDEBUG("[LOG]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define LOGDEBUG(a, ...)
#endif

#define LOGTRACE(a, ...) { TRACETENABLE { DAVEDEBUG("[LOG]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); } }

#define LOGLTRACE(t,n,a,...) { TRACELENABLE(t,n) { DAVEDEBUG("[LOG]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); } }

#define LOGABNOR(a, ...) { DAVEDEBUG("[LOG Abnormal]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }

#define LOGLOG(a, ...) { DAVEDEBUG("[LOG]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }

#endif


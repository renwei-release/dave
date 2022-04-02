/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifndef __LOG_LOG_H__
#define __LOG_LOG_H__
#include "dave_base.h"

#ifdef __BASE_ALPHA_VERSION__
// #define LOG_DEBUG
#endif

#ifdef LOG_DEBUG
#define LOGDEBUG(a, ...) { DAVEDEBUG("[LOG]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define LOGDEBUG(a, ...)
#endif

#define LOGABNOR(a, ...) { DAVEABNORMAL("[LOG Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define LOGLOG(a, ...) { DAVEDEBUG("[LOG]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }

#endif


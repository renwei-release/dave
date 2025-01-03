/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RTC_LOG_H__
#define __RTC_LOG_H__
#include "dave_base.h"

#if defined(LEVEL_PRODUCT_alpha) || defined(LEVEL_PRODUCT_beta)
// #define RTC_DEBUG
#endif

#ifdef RTC_DEBUG
#define RTCDEBUG(a, ...) { DAVEDEBUG("[RTC]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); } }
#else
#define RTCDEBUG(a, ...)
#endif

#define RTCTRACE(a, ...) { TRACETENABLE{ DAVETRACE("[RTC]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define RTCLTRACE(t,n,a,...) { TRACELENABLE(t,n) { DAVETRACE("[RTC]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define RTCABNOR(a, ...) { DAVEABNORMAL("[RTC Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define RTCLOG(a, ...) { DAVELOG("[RTC]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


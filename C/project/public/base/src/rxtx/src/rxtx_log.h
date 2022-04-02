/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RXTX_LOG_H__
#define __RXTX_LOG_H__
#include "base_macro.h"
#include "base_log.h"

#if defined(__BASE_ALPHA_VERSION__)
// #define RXTX_DEBUG
#endif

#ifdef RXTX_DEBUG
#define RTDEBUG(a, ...) { DAVEDEBUG("[RT]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define RTDEBUG(a, ...)
#endif

#define RTTRACE(a, ...) { TRACETENABLE { DAVETRACE("[RT]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define RTLTRACE(t,n,a,...) { TRACELENABLE(t,n) { DAVETRACE("[RT]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define RTABNOR(a, ...) { DAVEABNORMAL("[RT Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define RTLOG(a, ...) { DAVELOG("[RT]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


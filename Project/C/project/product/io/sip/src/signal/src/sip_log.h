/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SIP_LOG_H__
#define __SIP_LOG_H__
#include "dave_base.h"

#if defined(LEVEL_PRODUCT_alpha) || defined(LEVEL_PRODUCT_beta)
// #define SIP_DEBUG
#endif

#ifdef SIP_DEBUG
#define SIPDEBUG(a, ...) { DAVEDEBUG("[SIP]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); } }
#else
#define SIPDEBUG(a, ...)
#endif

#define SIPTRACE(a, ...) { TRACETENABLE{ DAVETRACE("[SIP]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define SIPLTRACE(t,n,a,...) { TRACELENABLE(t,n) { DAVETRACE("[SIP]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define SIPABNOR(a, ...) { DAVEABNORMAL("[SIP Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define SIPLOG(a, ...) { DAVELOG("[SIP]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


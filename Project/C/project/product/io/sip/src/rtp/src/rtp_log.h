/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RTP_LOG_H__
#define __RTP_LOG_H__
#include "dave_base.h"

#if defined(LEVEL_PRODUCT_alpha) || defined(LEVEL_PRODUCT_beta)
// #define RTP_DEBUG
#endif

#ifdef RTP_DEBUG
#define RTPDEBUG(a, ...) { DAVEDEBUG("[RTP]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); } }
#else
#define RTPDEBUG(a, ...)
#endif

#define RTPTRACE(a, ...) { TRACETENABLE{ DAVETRACE("[RTP]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define RTPLTRACE(t,n,a,...) { TRACELENABLE(t,n) { DAVETRACE("[RTP]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define RTPABNOR(a, ...) { DAVEABNORMAL("[RTP Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define RTPLOG(a, ...) { DAVELOG("[RTP]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


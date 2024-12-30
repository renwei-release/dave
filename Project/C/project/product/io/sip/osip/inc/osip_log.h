/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __OSIP_LOG_H__
#define __OSIP_LOG_H__
#include "dave_base.h"

#if defined(LEVEL_PRODUCT_alpha) || defined(LEVEL_PRODUCT_beta)
// #define OSIP_DEBUG
#endif

#ifdef OSIP_DEBUG
#define OSIPDEBUG(a, ...) { DAVEDEBUG("[OSIP]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); } }
#else
#define OSIPDEBUG(a, ...)
#endif

#define OSIPTRACE(a, ...) { TRACETENABLE{ DAVETRACE("[OSIP]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define OSIPLTRACE(t,n,a,...) { TRACELENABLE(t,n) { DAVETRACE("[OSIP]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define OSIPABNOR(a, ...) { DAVEABNORMAL("[OSIP Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define OSIPLOG(a, ...) { DAVELOG("[OSIP]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


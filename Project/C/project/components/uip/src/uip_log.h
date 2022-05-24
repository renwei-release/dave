/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UIP_LOG_H__
#define __UIP_LOG_H__
#include "dave_base.h"

#if defined(LEVEL_PRODUCT_alpha) || defined(LEVEL_PRODUCT_beta)
// #define UIP_DEBUG
#endif

#ifdef UIP_DEBUG
#define UIPDEBUG(a, ...) { DAVEDEBUG("[UIP]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); } }
#else
#define UIPDEBUG(a, ...)
#endif

#define UIPTRACE(a, ...) { TRACETENABLE{ DAVETRACE("[UIP]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define UIPLTRACE(t,n,a,...) { TRACELENABLE(t,n) { DAVETRACE("[UIP]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define UIPABNOR(a, ...) { DAVEABNORMAL("[UIP Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define UIPLOG(a, ...) { DAVELOG("[UIP]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


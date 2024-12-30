/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UAC_LOG_H__
#define __UAC_LOG_H__
#include "dave_base.h"

#if defined(LEVEL_PRODUCT_alpha) || defined(LEVEL_PRODUCT_beta)
// #define UAC_DEBUG
#endif

#ifdef UAC_DEBUG
#define UACDEBUG(a, ...) { DAVEDEBUG("[UAC]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); } }
#else
#define UACDEBUG(a, ...)
#endif

#define UACTRACE(a, ...) { TRACETENABLE{ DAVETRACE("[UAC]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define UACLTRACE(t,n,a,...) { TRACELENABLE(t,n) { DAVETRACE("[UAC]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define UACABNOR(a, ...) { DAVEABNORMAL("[UAC Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define UACLOG(a, ...) { DAVELOG("[UAC]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


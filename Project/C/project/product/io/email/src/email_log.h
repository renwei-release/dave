/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __EMAIL_LOG_H__
#define __EMAIL_LOG_H__
#include "dave_base.h"

#if defined(LEVEL_PRODUCT_alpha)
// #define EMAIL_DEBUG
#endif

#ifdef EMAIL_DEBUG
#define EMAILDEBUG(a, ...) { DAVEDEBUG("[EMAIL]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define EMAILDEBUG(a, ...)
#endif

#define EMAILTRACE(a, ...) { TRACETENABLE { DAVETRACE("[EMAIL]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define EMAILLTRACE(t,n,a,...) { TRACELENABLE(t,n) { DAVETRACE("[EMAIL]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define EMAILABNOR(a, ...) { DAVEABNORMAL("[EMAIL Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define EMAILLOG(a, ...) { DAVELOG("[EMAIL]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\r\n"); }

#endif


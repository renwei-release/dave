/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __HTTP_LOG_H__
#define __HTTP_LOG_H__
#include "dave_base.h"

#if defined(LEVEL_PRODUCT_alpha)
// #define HTTP_DEBUG
#endif

#ifdef HTTP_DEBUG
#define HTTPDEBUG(a, ...) { DAVEDEBUG("[HTTP]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\r\n"); }
#else
#define HTTPDEBUG(a, ...)
#endif

#define HTTPTRACE(a, ...) { TRACETENABLE { DAVETRACE("[HTTP]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\r\n"); } }

#define HTTPLTRACE(t,n,a,...) { TRACELENABLE(t,n) { DAVETRACE("[HTTP]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\r\n"); } }

#define HTTPABNOR(a, ...) { DAVEABNORMAL("[HTTP Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\r\n"); }

#define HTTPLOG(a, ...) { DAVELOG("[HTTP]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\r\n"); }

#endif


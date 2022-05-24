/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifndef __SOCKET_LOG_H__
#define __SOCKET_LOG_H__
#include "base_log.h"

#if defined(LEVEL_PRODUCT_alpha)
// #define SOCKET_DEBUG
#endif

#ifdef SOCKET_DEBUG
#define SOCKETDEBUG(a, ...) { DAVEDEBUG("[SOCKET]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define SOCKETDEBUG(a, ...)
#endif

#define SOCKETTRACE(a, ...) { TRACETENABLE { DAVETRACE("[SOCKET]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define SOCKETLTRACE(t,n,a,...) { TRACELENABLE(t,n) { DAVETRACE("[SOCKET]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define SOCKETABNOR(a, ...) { DAVEABNORMAL("[SOCKET Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define SOCKETLOG(a, ...) { DAVELOG("[SOCKET]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


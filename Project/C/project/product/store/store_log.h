/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __STORE_LOG_H__
#define __STORE_LOG_H__
#include "dave_base.h"

#ifdef __DAVE_ALPHA_VERSION__
// #define STORE_DEBUG
#endif

#ifdef STORE_DEBUG
#define STDEBUG(a, ...) { DAVEDEBUG("[STORE]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define STDEBUG(a, ...)
#endif

#define STTRACE(a, ...) { TRACETENABLE { DAVETRACE("[STORE]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define STABNOR(a, ...) { DAVEABNORMAL("[STORE Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define STLOG(a, ...) { DAVELOG("[STORE]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


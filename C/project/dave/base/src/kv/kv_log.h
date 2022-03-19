/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __KV_LOG_H__
#define __KV_LOG_H__
#include "dave_base.h"

#if defined(__BASE_ALPHA_VERSION__)
// #define KV_DEBUG
#endif

#ifdef KV_DEBUG
#define KVDEBUG(a, ...) { DAVEDEBUG("[KV]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define KVDEBUG(a, ...)
#endif

#define KVTRACE(a, ...) { TRACETENABLE { DAVETRACE("[KV]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define KVLTRACE(t,n,a, ...) { TRACELENABLE(t,n) { DAVETRACE("[KV]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define KVABNOR(a, ...) { DAVEABNORMAL("[KV Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define KVLOG(a, ...) { DAVELOG("[KV]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


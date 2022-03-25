/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DLL_LOG_H__
#define __DLL_LOG_H__
#include "dave_base.h"

#if defined(__BASE_ALPHA_VERSION__)
// #define DLL_DEBUG
#endif

#ifdef DLL_DEBUG
#define DLLDEBUG(a, ...) { DAVETRACE("[DLL]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); }
#else
#define DLLDEBUG(a, ...)
#endif

#define DLLABNOR(a, ...) { DAVEABNORMAL("[DLL Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define DLLLOG(a, ...) { DAVELOG("[DLL]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


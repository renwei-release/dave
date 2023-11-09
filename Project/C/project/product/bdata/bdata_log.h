/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BDATA_LOG_H__
#define __BDATA_LOG_H__
#include "dave_base.h"

#ifdef LEVEL_PRODUCT_alpha
// #define BDATA_DEBUG
#endif

#ifdef BDATA_DEBUG
#define BDDEBUG(a, ...) { DAVEDEBUG("[BDATA]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define BDDEBUG(a, ...)
#endif

#define BDTRACE(a, ...) { TRACETENABLE { DAVETRACE("[BDATA]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define BDABNOR(a, ...) { DAVEABNORMAL("[BDATA Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define BDLOG(a, ...) { DAVELOG("[BDATA]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef ___BASE_LOG_H___
#define ___BASE_LOG_H___
#include "dave_base.h"

#if defined(LEVEL_PRODUCT_alpha)
// #define BASE_DEBUG
#endif

#ifdef BASE_DEBUG
#define BASEDEBUG(a, ...) { DAVETRACE("[BASE]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); }
#else
#define BASEDEBUG(a, ...)
#endif

#define BASETRACE(a, ...) { TRACETENABLE{ DAVETRACE("[BASE]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define BASEABNOR(a, ...) { DAVEABNORMAL("[BASE Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define BASELOG(a, ...) { DAVELOG("[BASE]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


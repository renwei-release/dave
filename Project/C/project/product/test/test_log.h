/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef ___TEST_LOG_H___
#define ___TEST_LOG_H___
#include "dave_base.h"

#if defined(LEVEL_PRODUCT_alpha)
// #define TEST_DEBUG
#endif

#ifdef TEST_DEBUG
#define TESTDEBUG(a, ...) { DAVETRACE("[TEST]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); }
#else
#define TESTDEBUG(a, ...)
#endif

#define TESTTRACE(a, ...) { TRACETENABLE{ DAVETRACE("[TEST]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define TESTABNOR(a, ...) { DAVEABNORMAL("[TEST Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define TESTLOG(a, ...) { DAVELOG("[TEST]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


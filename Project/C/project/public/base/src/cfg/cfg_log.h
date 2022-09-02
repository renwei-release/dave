/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __CFG_LOG_H__
#define __CFG_LOG_H__
#include "base_macro.h"

#ifdef LEVEL_PRODUCT_alpha
// #define CFG_DEBUG
#endif

#ifdef CFG_DEBUG
#define CFGDEBUG(a, ...) { DAVEDEBUG("[CFG]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define CFGDEBUG(a, ...) {}
#endif

#define CFGLOG(a, ...) { DAVELOG("[CFG]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#define CFGABNOR(a, ...) { DAVEDEBUG("[CFG Abnormal]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }

#endif


/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __CFG_LOG_H__
#define __CFG_LOG_H__
#include "base_macro.h"

#ifdef __BASE_ALPHA_VERSION__
// #define CFG_DEBUG
#endif

#ifdef CFG_DEBUG
#define CFGDEBUG(a, ...) { DAVEDEBUG("[CFG]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define CFGDEBUG(a, ...) {}
#endif

#define CFGABNOR(a, ...) { DAVEDEBUG("[CFG Abnormal]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }

#endif


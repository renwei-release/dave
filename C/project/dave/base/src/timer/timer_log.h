/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __TIMER_LOG_H__
#define __TIMER_LOG_H__
#include "dave_base.h"

#ifdef __BASE_ALPHA_VERSION__
// #define TIMER_DEBUG
#endif

#ifdef TIMER_DEBUG
#define TIMEDEBUG(a, ...) { DAVEDEBUG("[TIME]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define TIMEDEBUG(a, ...) {}
#endif

#define TIMEABNOR(a, ...) { DAVEDEBUG("[TIME Abnormal]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }

#endif


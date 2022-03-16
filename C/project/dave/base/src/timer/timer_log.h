/*
 * ================================================================================
 * (c) Copyright 2016 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#ifndef __TIMER_LOG_H__
#define __TIMER_LOG_H__
#include "dave_base.h"

#ifdef __DAVE_ALPHA_VERSION__
// #define TIMER_DEBUG
#endif

#ifdef TIMER_DEBUG
#define TIMEDEBUG(a, ...) { DAVEDEBUG("[TIME]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define TIMEDEBUG(a, ...) {}
#endif

#define TIMEABNOR(a, ...) { DAVEDEBUG("[TIME Abnormal]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }

#endif


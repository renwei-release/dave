/*
 * ================================================================================
 * (c) Copyright 2020 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.08.14.
 * ================================================================================
 */

#include "base_macro.h"
#ifndef __OS_LOG_H__
#define __OS_LOG_H__
#include "dave_base.h"

#if defined(__DAVE_ALPHA_VERSION__)
// #define OS_DEBUG
#endif

#ifdef OS_DEBUG
#define OSDEBUG(a, ...) { DAVEDEBUG("[OS]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define OSDEBUG(a, ...)
#endif

#define OSABNOR(a, ...) { DAVEABNORMAL("[OS Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define OSLOG(a, ...) { DAVELOG("[OS]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


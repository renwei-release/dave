/*
 * ================================================================================
 * (c) Copyright 2016 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#ifndef __DOS_LOG_H__
#define __DOS_LOG_H__
#include "dave_base.h"

#ifdef __BASE_ALPHA_VERSION__
// #define DOS_DEBUG
#endif

#ifdef DOS_DEBUG
#define DOSDEBUG(a, ...) { DAVEDEBUG("[DOS]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define DOSDEBUG(a, ...)
#endif

#define DOSTRACE(a, ...) { TRACETENABLE{ DAVETRACE("[UI]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define DOSLOG(a, ...) { DAVECATCHER("[UI]"); DAVECATCHER((const char*)a, ##__VA_ARGS__); DAVECATCHER("\n"); }

#endif


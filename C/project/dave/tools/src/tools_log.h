/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __TOOLS_LOG_H__
#define __TOOLS_LOG_H__
#include "tools_macro.h"
#include "dave_base.h"

#if defined(__TOOLS_ALPHA_VERSION__)
// #define TOOLS_DEBUG
#endif

#ifdef TOOLS_DEBUG
#define TOOLSDEBUG(a, ...) { DAVEDEBUG("[TOOLS]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define TOOLSDEBUG(a, ...)
#endif

#define TOOLSTRACE(a, ...) { TRACETENABLE { DAVETRACE("[TOOLS]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define TOOLSLTRACE(t,n,a,...) { TRACELENABLE(t,n) { DAVETRACE("[TOOLS]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define TOOLSABNOR(a, ...) { DAVEABNORMAL("[TOOLS Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define TOOLSLOG(a, ...) { DAVELOG("[TOOLS]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


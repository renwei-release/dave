/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DOS_LOG_H__
#define __DOS_LOG_H__
#include "dave_base.h"

#ifdef LEVEL_PRODUCT_alpha
// #define DOS_DEBUG
#endif

#ifdef DOS_DEBUG
#define DOSDEBUG(a, ...) { DAVEDEBUG("[DOS]<%s:%d>", __func__, __LINE__); DAVEDEBUG((const char*)a, ##__VA_ARGS__); DAVEDEBUG("\n"); }
#else
#define DOSDEBUG(a, ...)
#endif

#define DOSTRACE(a, ...) { TRACETENABLE{ DAVETRACE("[DOS]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define DOSLOG(a, ...) { DAVECATCHER("[DOS]"); DAVECATCHER((const char*)a, ##__VA_ARGS__); DAVECATCHER("\n"); }

#endif


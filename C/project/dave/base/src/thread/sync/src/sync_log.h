/*
 * ================================================================================
 * (c) Copyright 2018 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2018.01.22.
 * ================================================================================
 */

#ifndef __SYNC_LOG_H__
#define __SYNC_LOG_H__
#include "dave_base.h"

#if defined(__DAVE_ALPHA_VERSION__)
// #define SYNC_DEBUG
#endif

#ifdef SYNC_DEBUG
#define SYNCDEBUG(a, ...) { DAVETRACE("[SYNC]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); }
#else
#define SYNCDEBUG(a, ...)
#endif

#define SYNCTRACE(a, ...) { TRACETENABLE { DAVETRACE("[SYNC]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define SYNCLTRACE(t,n,a,...) { TRACELENABLE(t,n) { DAVETRACE("[SYNC]<%s:%d>", __func__, __LINE__); DAVETRACE((const char*)a, ##__VA_ARGS__); DAVETRACE("\n"); } }

#define SYNCABNOR(a, ...) { DAVEABNORMAL("[SYNC Abnormal]<%s:%d>", __func__, __LINE__); DAVEABNORMAL((const char*)a, ##__VA_ARGS__); DAVEABNORMAL("\n"); }

#define SYNCLOG(a, ...) { DAVELOG("[SYNC]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


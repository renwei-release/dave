/*
 * ================================================================================
 * (c) Copyright 2017 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2017.04.26.
 * The synchronization service in the future may try to connect to all within
 * the framework of the server.
 * ================================================================================
 */

#ifndef __THREAD_SYNC_H__
#define __THREAD_SYNC_H__
#include "base_macro.h"

#define SYNC_CLIENT_THREAD_NAME "syncc"
#define SYNC_SERVER_THREAD_NAME "syncs"

#if defined(__BASE_PRODUCT_SYNC__) || defined(__BASE_PRODUCT_BASE__)
#define SYNC_STACK_SERVER
#endif
#if !(defined(__BASE_PRODUCT_SYNC__) || defined(__BASE_PRODUCT_LOG__) || defined(__BASE_PRODUCT_DEBUG__))
#define SYNC_STACK_CLIENT
#endif

void thread_sync_init(void);

void thread_sync_exit(void);

#endif


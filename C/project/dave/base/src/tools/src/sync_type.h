/*
 * ================================================================================
 * (c) Copyright 2021 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2021.10.30.
 * ================================================================================
 */

#ifndef __SYNC_TYPE_H__
#define __SYNC_TYPE_H__

typedef enum {
	SYNC_SERVER,
	SYNC_CLIENT,
	SYNC_MAX,
} SYNCType;

SYNCType T_sync_type(void);

#endif


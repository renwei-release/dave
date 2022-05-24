/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
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


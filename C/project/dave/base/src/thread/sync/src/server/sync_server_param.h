/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_SERVER_PARAM_H__
#define __SYNC_SERVER_PARAM_H__
#include "sync_param.h"

typedef enum {
	SyncServerEvents_version,
	SyncServerEvents_link_up,
	SyncServerEvents_link_down,
} SyncServerEvents;

#define SYNC_SERVER_BASE_TIME (1000)
#define SYNC_CLIENT_LEFT_MAX (60)
#define SYNC_CLIENT_SYNC_MAX (2)
#define SYNC_MAX_RELEASE_QUANTITY (2048)

#endif


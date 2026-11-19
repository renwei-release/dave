/*
 * Copyright (c) 2026 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifndef __SYNC_CLIENT_LOST_LINK_H__
#define __SYNC_CLIENT_LOST_LINK_H__
#include "sync_param.h"

void sync_client_lost_link_in(SyncServer *pServer);
void sync_client_lost_link_out(SyncServer *pServer);

#endif
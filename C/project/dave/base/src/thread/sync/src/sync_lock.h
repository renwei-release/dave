/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_LOCK_H__
#define __SYNC_LOCK_H__

void sync_lock_init(void);

void sync_lock_exit(void);

void sync_lock(void);

void sync_unlock(void);

#endif


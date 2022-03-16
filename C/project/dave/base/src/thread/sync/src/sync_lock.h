/*
 * ================================================================================
 * (c) Copyright 2018 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2018.07.12.
 * ================================================================================
 */

#ifndef __SYNC_LOCK_H__
#define __SYNC_LOCK_H__

void sync_lock_init(void);

void sync_lock_exit(void);

void sync_lock(void);

void sync_unlock(void);

#endif


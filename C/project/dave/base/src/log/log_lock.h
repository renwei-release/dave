/*
 * ================================================================================
 * (c) Copyright 2018 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2018.06.25.
 * ================================================================================
 */

#ifndef __LOG_LOCK_H__
#define __LOG_LOCK_H__

void log_lock_init(void);

void log_lock_exit(void);

void log_lock(void);

void log_unlock(void);

#endif


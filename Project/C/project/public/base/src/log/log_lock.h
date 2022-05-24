/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __LOG_LOCK_H__
#define __LOG_LOCK_H__

void log_lock_init(void);

void log_lock_exit(void);

void log_lock(void);

void log_unlock(void);

#endif


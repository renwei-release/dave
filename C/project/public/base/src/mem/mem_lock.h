/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __MEM_LOCK_H__
#define __MEM_LOCK_H__

void mem_lock_init(void);

void mem_lock_exit(void);

void mem_lock(void);

void mem_unlock(void);

#endif


/*
 * ================================================================================
 * (c) Copyright 2018 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2018.06.25.
 * ================================================================================
 */

#ifndef __MEM_LOCK_H__
#define __MEM_LOCK_H__

void mem_lock_init(void);

void mem_lock_exit(void);

void mem_lock(void);

void mem_unlock(void);

#endif


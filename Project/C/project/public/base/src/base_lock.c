/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_os.h"
#include "dave_tools.h"

extern void log_lock_init(void);
extern void log_lock_exit(void);
extern void thread_lock_init(void);
extern void thread_lock_exit(void);
extern void mem_lock_init(void);
extern void mem_lock_exit(void);

static TLock _t_base_lock;

// =====================================================================

void
booting_lock(void)
{
	t_lock_booting();
	log_lock_init();
	thread_lock_init();
	mem_lock_init();
	t_lock_reset(&_t_base_lock);
}

void
base_lock(void)
{
	t_lock_spin(&_t_base_lock);
}

void
base_unlock(void)
{
	t_unlock_spin(&_t_base_lock);
}

#endif


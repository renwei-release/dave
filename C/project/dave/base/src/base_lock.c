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

// =====================================================================

void
booting_lock(void)
{
	t_lock_booting();
	log_lock_init();
	thread_lock_init();
	mem_lock_init();
}

#endif


/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "dave_tools.h"
#include "dave_os.h"

static TLock _log_pv;

static inline void
_log_lock_init(void)
{
	static volatile sb __safe_pre_flag__ = 0;

	SAFEPre(__safe_pre_flag__, t_lock_reset(&_log_pv); );
}

// =====================================================================

void
log_lock_init(void)
{
	_log_lock_init();
}

void
log_lock_exit(void)
{
/*
 *  Need logs can be available at any time!
 *
 *	dave_os_spin_lock_destroy((void *)_lock_ptr);
 */
}

void
log_lock(void)
{
	_log_lock_init();

	t_lock_spin(&_log_pv);
}

void
log_unlock(sb flag)
{
	t_unlock_spin(&_log_pv);
}


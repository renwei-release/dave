/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "dave_os.h"

static ub _lock_ptr_init_flag = 0x00;
static s8 _lock_ptr[256];

static inline void
_log_lock_init(void)
{
	if(_lock_ptr_init_flag != 0x1234567890)
	{
		_lock_ptr_init_flag = 0x1234567890;
		dave_os_spin_lock_init((void *)_lock_ptr);
	}
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
	dave_os_spin_lock((void *)_lock_ptr);
}

void
log_unlock(sb flag)
{
	dave_os_spin_unlock((void *)_lock_ptr);
}


/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_tools.h"
#include "dave_base.h"
#include "dave_os.h"
#include "base_log.h"

static s8 _lock_ptr[256];
static s8 _exter_lock_ptr[256];

// =====================================================================

void
mem_lock_init(void)
{
	dave_os_spin_lock_init((void *)_lock_ptr);
	dave_os_spin_lock_init((void *)_exter_lock_ptr);
}

void
mem_lock_exit(void)
{
	dave_os_spin_lock_destroy((void *)_lock_ptr);
	dave_os_spin_lock_destroy((void *)_exter_lock_ptr);
}

void
mem_lock(void)
{
	dave_os_spin_lock((void *)_lock_ptr);
}

void
mem_unlock(void)
{
	dave_os_spin_unlock((void *)_lock_ptr);
}

#endif


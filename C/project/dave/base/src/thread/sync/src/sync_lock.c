/*
 * ================================================================================
 * (c) Copyright 2018 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2018.06.25.
 * ================================================================================
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_CLIENT) || defined(SYNC_STACK_SERVER)
#include "dave_base.h"
#include "dave_tools.h"
#include "sync_lock.h"

static s8 _lock_ptr[256];

// =====================================================================

void
sync_lock_init(void)
{
	dave_os_spin_lock_init((void *)_lock_ptr);
}

void
sync_lock_exit(void)
{
	dave_os_spin_lock_destroy((void *)_lock_ptr);
}

void
sync_lock(void)
{
	dave_os_spin_lock((void *)_lock_ptr);
}

void
sync_unlock(void)
{
	dave_os_spin_unlock((void *)_lock_ptr);
}

#endif


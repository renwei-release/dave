/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"

static TLock _uac_global_pv;

// =====================================================================

void
uac_global_lock_init(void)
{
	t_lock_reset(&_uac_global_pv);
}

void
uac_global_lock_exit(void)
{

}

void
uac_global_lock(void)
{
	t_lock_spin(&_uac_global_pv);
}

void
uac_global_unlock(void)
{
	t_unlock_spin(&_uac_global_pv);
}


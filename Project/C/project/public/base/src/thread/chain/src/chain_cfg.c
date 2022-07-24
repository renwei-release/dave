/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "base_define.h"
#include "base_tools.h"
#include "base_lock.h"

#define CFG_BASE_CHAIN_ENABLE "BaseChainEnable"

static dave_bool _chain_enable = dave_true;

// =====================================================================

void
chain_cfg_reset(void)
{
	_chain_enable = cfg_get_bool(CFG_BASE_CHAIN_ENABLE, _chain_enable);
}

dave_bool
chain_enable(void)
{
	return _chain_enable;
}

#endif


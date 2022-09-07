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
#include "thread_log.h"

static dave_bool _chain_enable = dave_false;

static void
_chain_cfg_reset(CFGUpdate *pUpdate)
{
	dave_bool chain_enable;

	if(pUpdate == NULL)
	{
		_chain_enable = cfg_get_bool(CFG_BASE_CHAIN_ENABLE, _chain_enable);
		return;
	}

	if(dave_strcmp(pUpdate->cfg_name, CFG_BASE_CHAIN_ENABLE) == dave_true)
	{
		chain_enable = cfg_get_bool(CFG_BASE_CHAIN_ENABLE, _chain_enable);
		if(chain_enable != _chain_enable)
		{
			_chain_enable = chain_enable;

			cfg_set_bool(CFG_BASE_CHAIN_ENABLE, _chain_enable);

			THREADLOG("chain state:%s!", _chain_enable==dave_true?"enable":"disable");
		}
	}
}

// =====================================================================

void
chain_cfg_reset(CFGUpdate *pUpdate)
{
	_chain_cfg_reset(pUpdate);

	chain_config_reset(pUpdate);
}

dave_bool
chain_enable(void)
{
	return _chain_enable;
}

dave_bool
chain_type_enable(ChainType type)
{
	return chain_config_type_enable(type);
}

#endif


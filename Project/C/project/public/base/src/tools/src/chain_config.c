/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_tools.h"
#include "thread_chain.h"

typedef enum {
	ChainCfg_none = 0,
	ChainCfg_calling = 0x01,
	ChainCfg_called = 0x02,
	ChainCfg_execution = 0x04,
	ChainCfg_coroutine = 0x08,
	ChainCfg_max = 0xff
} ChainCfg;

static ChainCfg _chain_enable_cfg = ChainCfg_execution | ChainCfg_coroutine;

static void
_chain_config_reset(void)
{
	s8 cfg_data[1024];
	ub cfg_len, safe_counter;
	s8 *cfg_ptr;
	s8 type_str[128];

	cfg_get_by_default(CFG_BASE_CHAIN_TYPE, cfg_data, sizeof(cfg_data), "execution|coroutine");

	cfg_len = dave_strlen(cfg_data);
	safe_counter = 0;
	cfg_ptr = cfg_data;

	_chain_enable_cfg = ChainCfg_none;

	while(((safe_counter ++) <= cfg_len) && (cfg_ptr != NULL))
	{
		cfg_ptr = dave_strfind(cfg_ptr, '|', type_str, sizeof(type_str));
		if(type_str[0] == '\0')
			break;

		if(dave_strcmp(type_str, "calling") == dave_true)
		{
			_chain_enable_cfg |= ChainCfg_calling;
		}
		else if(dave_strcmp(type_str, "called") == dave_true)
		{
			_chain_enable_cfg |= ChainCfg_called;
		}
		else if(dave_strcmp(type_str, "execution") == dave_true)
		{
			_chain_enable_cfg |= ChainCfg_execution;
		}
		else if(dave_strcmp(type_str, "coroutine") == dave_true)
		{
			_chain_enable_cfg |= ChainCfg_coroutine;
		}
	}
}

static inline dave_bool
_chain_config_type_enable(ChainType type)
{
	dave_bool enable = dave_false;

	switch(type)
	{
		case ChainType_calling:
				enable = ((_chain_enable_cfg & ChainCfg_calling) == 0x00) ? dave_false : dave_true;
			break;
		case ChainType_called:
				enable = ((_chain_enable_cfg & ChainCfg_called) == 0x00) ? dave_false : dave_true;
			break;
		case ChainType_execution:
				enable = ((_chain_enable_cfg & ChainCfg_execution) == 0x00) ? dave_false : dave_true;
			break;
		case ChainType_coroutine:
				enable = ((_chain_enable_cfg & ChainCfg_coroutine) == 0x00) ? dave_false : dave_true;
			break;
		default:
			break;
	}

	return enable;
}

// =====================================================================

void
chain_config_reset(CFGUpdate *pUpdate)
{
	if((pUpdate == NULL)
		|| (dave_strcmp(pUpdate->cfg_name, CFG_BASE_CHAIN_TYPE) == dave_true))
	{
		_chain_config_reset();
	}
}

dave_bool
chain_config_type_enable(ChainType type)
{
	return _chain_config_type_enable(type);
}

#endif


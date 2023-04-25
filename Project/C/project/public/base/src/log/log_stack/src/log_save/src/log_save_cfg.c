/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "log_stack.h"
#if defined(LOG_STACK_SERVER) || defined(LOG_STACK_CLIENT)
#include <dlfcn.h>
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "base_tools.h"

#define CFG_LOG_SAVE_TYPE "LogSaveType"

typedef enum {
	LogSaveType_none = 0,
	LogSaveType_json = 0x01,
	LogSaveType_txt = 0x02,
	LogSaveType_max = 0xff
} LogSaveType;

static LogSaveType _log_save_type = LogSaveType_json|LogSaveType_txt;

static void
_log_save_type_reset(void)
{
	s8 cfg_data[1024];
	ub cfg_len, safe_counter;
	s8 *cfg_ptr;
	s8 type_str[128];

#ifdef LOG_STACK_SERVER
	cfg_get_by_default(CFG_LOG_SAVE_TYPE, cfg_data, sizeof(cfg_data), "json|txt");
#else
	cfg_get_by_default(CFG_LOG_SAVE_TYPE, cfg_data, sizeof(cfg_data), "txt");
#endif

	cfg_len = dave_strlen(cfg_data);
	safe_counter = 0;
	cfg_ptr = cfg_data;

	_log_save_type = LogSaveType_txt;

	while(((safe_counter ++) <= cfg_len) && (cfg_ptr != NULL))
	{
		cfg_ptr = dave_strfind(cfg_ptr, '|', type_str, sizeof(type_str));
		if(type_str[0] == '\0')
			break;

		if(dave_strcmp(type_str, "json") == dave_true)
		{
			_log_save_type |= LogSaveType_json;
		}
		else if(dave_strcmp(type_str, "txt") == dave_true)
		{
			_log_save_type |= LogSaveType_txt;
		}
	}
}

static void
_log_save_cfg_update(MSGBODY *msg)
{
	CFGUpdate *pUpdate = (CFGUpdate *)(msg->msg_body);

	chain_config_reset(pUpdate);

	if(dave_strcmp(pUpdate->cfg_name, CFG_LOG_SAVE_TYPE) == dave_true)
	{
		_log_save_type_reset();
	}
}

// =====================================================================

void
log_save_cfg_init(void)
{
	chain_config_reset(NULL);

	_log_save_type_reset();

	reg_msg(MSGID_CFG_UPDATE, _log_save_cfg_update);
}

void
log_save_cfg_exit(void)
{
	unreg_msg(MSGID_CFG_UPDATE);
}

dave_bool
log_save_type_enable(ChainType type)
{
	return chain_config_type_enable(type);
}

dave_bool
log_save_json_enable(void)
{
	if(_log_save_type & LogSaveType_json)
		return dave_true;
	else
		return dave_false;
}

dave_bool
log_save_txt_enable(void)
{
	if(_log_save_type & LogSaveType_txt)
		return dave_true;
	else
		return dave_false;
}

#endif


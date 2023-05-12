/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_tools.h"
#include "cfg_log.h"

#define REG_CFG_MAX 32

typedef struct {
	s8 name[512];
	ThreadId reg_id[REG_CFG_MAX];
	cfg_reg_fun reg_fun[REG_CFG_MAX];
} RegCfgData;

static void *_reg_cfg_kv = NULL;
static TLock _reg_cfg_pv;

static RegCfgData *
_reg_cfg_data_malloc(s8 *name)
{
	RegCfgData *pData = dave_ralloc(sizeof(RegCfgData));
	ub reg_index;

	dave_strcpy(pData->name, name, sizeof(pData->name));
	for(reg_index=0; reg_index<REG_CFG_MAX; reg_index++)
	{
		pData->reg_id[reg_index] = INVALID_THREAD_ID;
		pData->reg_fun[reg_index] = NULL;
	}

	return pData;
}

static void
_reg_cfg_data_free(RegCfgData *pData)
{
	if(pData != NULL)
	{
		dave_free(pData);
	}
}

static dave_bool
_reg_cfg_reg(s8 *name, cfg_reg_fun reg_fun)
{
	RegCfgData *pData = kv_inq_key_ptr(_reg_cfg_kv, name);
	ub reg_index;

	if(pData == NULL)
	{
		pData = _reg_cfg_data_malloc(name);
	}

	for(reg_index=0; reg_index<REG_CFG_MAX; reg_index++)
	{
		if(pData->reg_fun[reg_index] == NULL)
		{
			pData->reg_id[reg_index] = self();
			pData->reg_fun[reg_index] = reg_fun;
			break;
		}
	}
	if(reg_index >= REG_CFG_MAX)
	{
		CFGLOG("reg_index if full on name:%s", name);
		return dave_false;
	}

	return kv_add_key_ptr(_reg_cfg_kv, name, pData);
}

static RetCode
_reg_cfg_recycle(void *ramkv, s8 *name)
{
	RegCfgData *pData = (RegCfgData *)kv_del_key_ptr(_reg_cfg_kv, name);

	if(pData == NULL)
		return RetCode_empty_data;

	_reg_cfg_data_free(pData);

	return RetCode_OK;
}

static void
_reg_cfg_update(MSGBODY *msg)
{
	CFGUpdate *pUpdate = (CFGUpdate *)(msg->msg_body);
	RegCfgData *pData = kv_inq_key_ptr(_reg_cfg_kv, pUpdate->cfg_name);
	ub reg_index;
	ThreadId self_id;
	s8 *cfg_name_ptr = pUpdate->cfg_name;
	s8 *cfg_value_ptr = (s8 *)(pUpdate->cfg_value);
	ub cfg_name_len, cfg_value_len;

	if(pData == NULL)
	{
		CFGDEBUG("%s not process %s:%s",
			thread_name(msg->msg_dst),
			pUpdate->cfg_name, pUpdate->cfg_value);
		return;
	}

	self_id = self();
	cfg_name_len = dave_strlen(cfg_name_ptr);
	cfg_value_len = dave_strlen(cfg_value_ptr);

	for(reg_index=0; reg_index<REG_CFG_MAX; reg_index++)
	{
		if(pData->reg_fun[reg_index] == NULL)
			break;

		CFGDEBUG("reg_index:%d reg_id:%s self_id:%s %s:%s",
			reg_index, thread_name(pData->reg_id[reg_index]), thread_name(self_id),
			cfg_name_ptr, cfg_value_ptr);

		if(pData->reg_id[reg_index] == self_id)
		{
			pData->reg_fun[reg_index](cfg_name_ptr, cfg_name_len, cfg_value_ptr, cfg_value_len);
		}
	}
}

static inline void
_reg_cfg_pre(void)
{
	static volatile sb __safe_pre_flag__ = 0;

	SAFEPre(__safe_pre_flag__, {
		_reg_cfg_kv = kv_malloc("regcfg", 0, NULL);
		t_lock_reset(&_reg_cfg_pv);
	});
}

// =====================================================================

void
reg_cfg_init(void)
{
	_reg_cfg_pre();
}

void
reg_cfg_exit(void)
{
	if(_reg_cfg_kv != NULL)
	{
		kv_free(_reg_cfg_kv, _reg_cfg_recycle);
		_reg_cfg_kv = NULL;
	}
}

dave_bool
reg_cfg_reg(s8 *name, cfg_reg_fun reg_fun)
{
	dave_bool ret = dave_false;

	_reg_cfg_pre();

	SAFECODEv1(_reg_cfg_pv, ret = _reg_cfg_reg(name, reg_fun));

	reg_msg(MSGID_CFG_UPDATE, _reg_cfg_update);

	return ret;
}

#endif


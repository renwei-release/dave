/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "dave_tools.h"
#include "cfg_param.h"
#include "cfg_log.h"

static void *_remote_cfg_kv = NULL;

// =====================================================================

void
base_remote_cfg_init(void)
{
	_remote_cfg_kv = kv_malloc("rcfgkv", KvAttrib_list, 0, NULL);
}

void
base_remote_cfg_exit(void)
{
	kv_free(_remote_cfg_kv, NULL);
}

dave_bool
base_remote_cfg_internal_add(s8 *name, s8 *value)
{
	return kv_add_key_value(_remote_cfg_kv, name, value);
}

dave_bool
base_remote_cfg_internal_del(s8 *name)
{
	return kv_del_key_value(_remote_cfg_kv, name);
}

RetCode
base_remote_cfg_set(s8 *name, s8 *value)
{
	CFGRemoteUpdate *pUpdate = thread_msg(pUpdate);

	pUpdate->put_flag = dave_true;
	dave_strcpy(pUpdate->cfg_name, name, sizeof(pUpdate->cfg_name));
	dave_strcpy(pUpdate->cfg_value, value, sizeof(pUpdate->cfg_value));

	name_msg(SYNC_CLIENT_THREAD_NAME, MSGID_CFG_REMOTE_UPDATE, pUpdate);

	return RetCode_OK;
}

sb
base_remote_cfg_get(s8 *name, s8 *value_ptr, ub value_len)
{
	return kv_inq_key_value(_remote_cfg_kv, name, value_ptr, value_len);
}

sb
base_remote_cfg_index(ub index, s8 *key_ptr, ub key_len, s8 *value_ptr, ub value_len)
{
	return kv_index_key_value(_remote_cfg_kv, index, key_ptr, key_len, value_ptr, value_len);
}

#endif


/*
 * Copyright (c) 2023 Renwei
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
remote_cfg_kv_init(void)
{
	_remote_cfg_kv = kv_malloc("rcfgkv", 0, NULL);
}

void
remote_cfg_kv_exit(void)
{
	kv_free(_remote_cfg_kv, NULL);
}

dave_bool
remote_cfg_kv_set(s8 *name, s8 *value)
{
	CFGDEBUG("%s : %s", name, value);
	return kv_add_key_value(_remote_cfg_kv, name, value);
}

dave_bool
remote_cfg_kv_del(s8 *name)
{
	return kv_del_key_value(_remote_cfg_kv, name);
}

sb
remote_cfg_kv_get(s8 *name, s8 *value_ptr, ub value_len)
{
	return kv_inq_key_value(_remote_cfg_kv, name, value_ptr, value_len);
}

sb
remote_cfg_kv_index(ub index, s8 *key_ptr, ub key_len, s8 *value_ptr, ub value_len)
{
	return kv_index_key_value(_remote_cfg_kv, index, key_ptr, key_len, value_ptr, value_len);
}

#endif


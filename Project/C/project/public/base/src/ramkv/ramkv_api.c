/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_os.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "ramkv_param.h"
#include "ramkv_struct.h"
#include "ramkv_api.h"
#include "ramkv_timer.h"
#include "ramkv_local_api.h"
#include "ramkv_remote_api.h"
#include "ramkv_log.h"

// ====================================================================

void
ramkv_api_init(void)
{

}

void
ramkv_api_exit(void)
{

}

dave_bool
ramkv_add(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	dave_bool ret;

	if(__ramkv_check__(pKV, fun, line) == dave_false)
		return dave_false;

	if((key_ptr == NULL) || (key_len == 0))
	{
		KVLOG("invalid key:%x/%d <%s:%d>", key_ptr, key_len, fun, line);
		return dave_false;
	}
	if(value_ptr == NULL)
	{
		KVLOG("invalid value:%x/%d <%s:%d>", value_ptr, value_len, fun, line);
		return dave_false;
	}

	switch(pKV->attrib)
	{
		case KvAttrib_ram:
		case KvAttrib_list:
				ret = ramkv_local_add(pKV, key_ptr, key_len, value_ptr, value_len, fun, line);
			break;
		case KvAttrib_remote:
				ret = ramkv_remote_add(pKV->attrib, pKV, key_ptr, key_len, value_ptr, value_len);
			break;
		default:
				ret = dave_false;
			break;
	}

	if(pKV->ramkv_timer.out_times > 0)
	{
		ramkv_timer_add(pKV, key_ptr, key_len);
	}

	return ret;
}

sb
ramkv_inq(KV *pKV, sb index, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	sb ret;

	if((value_ptr != NULL) && (value_len > 0))
	{
		((s8 *)value_ptr)[0] = '\0';
	}

	if(__ramkv_check__(pKV, fun, line) == dave_false)
		return -1;

	switch(pKV->attrib)
	{
		case KvAttrib_ram:
		case KvAttrib_list:
				ret = ramkv_local_inq(pKV, index, key_ptr, key_len, value_ptr, value_len, fun, line);
			break;
		case KvAttrib_remote:
				ret = ramkv_remote_inq(pKV->attrib, pKV, key_ptr, key_len, value_ptr, value_len);
			break;
		default:
				ret = -1;
			break;
	}

	if((pKV->ramkv_timer.out_times > 0) && (pKV->ramkv_timer.inq_update_timer == dave_true))
	{
		ramkv_timer_inq(pKV, key_ptr, key_len);
	}

	return ret;
}

ub
ramkv_del(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	ub ret;

	if(__ramkv_check__(pKV, fun, line) == dave_false)
		return 0;

	switch(pKV->attrib)
	{
		case KvAttrib_ram:
		case KvAttrib_list:
				ret = ramkv_local_del(pKV, key_ptr, key_len, value_ptr, value_len, fun, line);
			break;
		case KvAttrib_remote:
				ret = ramkv_remote_del(pKV->attrib, pKV, key_ptr, key_len);
			break;
		default:
				ret = 0;
			break;
	}

	if((ret != 0) && (pKV->ramkv_timer.out_times > 0))
	{
		ramkv_timer_del(pKV, key_ptr, key_len, fun, line);
	}

	if((ret == 0) && (key_ptr != NULL))
	{
		KVTRACE("On %s found repeated deletion or negligent deletion of KEY(%x/%x) at position %s:%d",
			pKV->name, key_ptr[0], key_ptr[1],
			fun, line);
	}

	return ret;
}

dave_bool
ramkv_top(KV *pKV, u8 *key_ptr, ub key_len)
{
	dave_bool ret;

	if(ramkv_check(pKV) == dave_false)
		return dave_false;

	switch(pKV->attrib)
	{
		case KvAttrib_ram:
		case KvAttrib_list:
				ret = ramkv_local_top(pKV, key_ptr, key_len);
			break;
		default:
				KVLOG("unsupport attrib:%d", pKV->attrib);
				ret = dave_false;
			break;
	}

	return ret;
}

ub
ramkv_info(KV *pKV, s8 *info_ptr, ub info_len)
{
	if(ramkv_check(pKV) == dave_false)
		return 0;

	return ramkv_timer_info(pKV, info_ptr, info_len);
}

#endif


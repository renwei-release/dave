/*
 * ================================================================================
 * (c) Copyright 2020 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.10.22.
 * ================================================================================
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_os.h"
#include "dave_tools.h"
#include "kv_param.h"
#include "kv_struct.h"
#include "kv_api.h"
#include "kv_timer.h"
#include "kv_local_api.h"
#include "kv_remote_api.h"
#include "kv_log.h"

// ====================================================================

void
kv_api_init(void)
{

}

void
kv_api_exit(void)
{

}

dave_bool
kv_add(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	dave_bool ret;

	switch(pKV->attrib)
	{
		case KVAttrib_ram:
		case KVAttrib_list:
				ret = kv_local_add(pKV, key_ptr, key_len, value_ptr, value_len, fun, line);
			break;
		case KVAttrib_remote:
				ret = kv_remote_add(pKV->attrib, pKV, key_ptr, key_len, value_ptr, value_len);
			break;
		default:
				ret = dave_false;
			break;
	}

	if(pKV->kv_timer.out_times > 0)
	{
		kv_timer_add(pKV, key_ptr, key_len);
	}

	return ret;
}

ub
kv_inq(KV *pKV, sb index, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	ub ret;

	switch(pKV->attrib)
	{
		case KVAttrib_ram:
		case KVAttrib_list:
				ret = kv_local_inq(pKV, index, key_ptr, key_len, value_ptr, value_len, fun, line);
			break;
		case KVAttrib_remote:
				ret = kv_remote_inq(pKV->attrib, pKV, key_ptr, key_len, value_ptr, value_len);
			break;
		default:
				ret = dave_false;
			break;
	}

	if((pKV->kv_timer.out_times > 0) && (pKV->kv_timer.inq_update_timer == dave_true))
	{
		kv_timer_inq(pKV, key_ptr, key_len);
	}

	return ret;
}

ub
kv_del(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	ub ret;

	switch(pKV->attrib)
	{
		case KVAttrib_ram:
		case KVAttrib_list:
				ret = kv_local_del(pKV, key_ptr, key_len, value_ptr, value_len, fun, line);
			break;
		case KVAttrib_remote:
				ret = kv_remote_del(pKV->attrib, pKV, key_ptr, key_len);
			break;
		default:
				ret = dave_false;
			break;
	}

	if(pKV->kv_timer.out_times > 0)
	{
		kv_timer_del(pKV, key_ptr, key_len, fun, line);
	}

	if((ret == 0) && (key_ptr != NULL))
	{
		KVLTRACE(60,1,"On %s found repeated deletion or negligent deletion of KEY(%x/%x) at position %s:%d",
			pKV->name, key_ptr[0], key_ptr[1],
			fun, line);
	}

	return ret;
}

dave_bool
kv_top(KV *pKV, u8 *key_ptr, ub key_len)
{
	dave_bool ret;

	switch(pKV->attrib)
	{
		case KVAttrib_ram:
		case KVAttrib_list:
				ret = kv_local_top(pKV, key_ptr, key_len);
			break;
		default:
				KVLOG("unsupport attrib:%d", pKV->attrib);
				ret = dave_false;
			break;
	}

	return ret;
}

ub
kv_info(KV *pKV, s8 *info_ptr, ub info_len)
{
	return kv_timer_info(pKV, info_ptr, info_len);
}

#endif


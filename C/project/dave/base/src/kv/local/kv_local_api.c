/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_os.h"
#include "dave_tools.h"
#include "kv_param.h"
#include "kv_local_struct.h"
#include "kv_local_multimap.h"
#include "kv_list_add.h"
#include "kv_list_inq.h"
#include "kv_list_del.h"
#include "kv_log.h"

// ====================================================================

dave_bool
kv_local_add(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	dave_bool ret;

	KVDEBUG("name:%s attrib:%d", pKV->name, pKV->attrib);

	switch(pKV->attrib)
	{
		case KVAttrib_ram:
				ret = kv_local_multimap_add(pKV, key_ptr, key_len, value_ptr, value_len, fun, line);
			break;
		case KVAttrib_list:
				ret = __kv_list_add__(pKV, key_ptr, key_len, value_ptr, value_len, fun, line);
			break;
		default:
				ret = dave_false;
			break;
	}

	return ret;
}

ub
kv_local_inq(KV *pKV, sb index, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	ub ret;

	KVDEBUG("name:%s attrib:%d", pKV->name, pKV->attrib);

	switch(pKV->attrib)
	{
		case KVAttrib_ram:
				ret = kv_local_multimap_inq(pKV, index, key_ptr, key_len, value_ptr, value_len);
			break;
		case KVAttrib_list:
				ret = __kv_list_inq__(pKV, index, key_ptr, key_len, value_ptr, value_len, fun, line);
			break;
		default:
				ret = 0;
			break;
	}

	return ret;
}

ub
kv_local_del(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	ub ret;

	KVDEBUG("name:%s attrib:%d %d <%s:%d>",
		pKV->name, pKV->attrib, key_len,
		fun, line);

	switch(pKV->attrib)
	{
		case KVAttrib_ram:
				ret = kv_local_multimap_del(pKV, key_ptr, key_len, value_ptr, value_len);
			break;
		case KVAttrib_list:
				ret =  __kv_list_del__(pKV, key_ptr, key_len, value_ptr, value_len, fun, line);
			break;
		default:
				ret = 0;
			break;
	}

	return ret;
}

dave_bool
kv_local_top(KV *pKV, u8 *key_ptr, ub key_len)
{
	dave_bool ret;

	switch(pKV->attrib)
	{
		case KVAttrib_ram:
				/*
				 * 不方便计算top。
				 */
				KVTRACE("unsupport attrib:%d", pKV->attrib);
				ret = dave_false;
			break;
		case KVAttrib_list:
				ret =  __kv_list_top__(pKV, key_ptr, key_len);
			break;
		default:
				KVLOG("unsupport attrib:%d", pKV->attrib);
				ret = dave_false;
			break;
	}

	return ret;
}

#endif


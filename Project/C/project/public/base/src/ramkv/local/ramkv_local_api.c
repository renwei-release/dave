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
#include "ramkv_param.h"
#include "ramkv_local_struct.h"
#include "ramkv_local_multimap.h"
#include "ramkv_list_add.h"
#include "ramkv_list_inq.h"
#include "ramkv_list_del.h"
#include "ramkv_log.h"

// ====================================================================

dave_bool
ramkv_local_add(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	dave_bool ret;

	KVDEBUG("name:%s attrib:%d", pKV->name, pKV->attrib);

	switch(pKV->attrib)
	{
		case KvAttrib_ram:
				ret = ramkv_local_multimap_add(pKV, key_ptr, key_len, value_ptr, value_len, fun, line);
			break;
		case KvAttrib_list:
				ret = __ramkv_list_add__(pKV, key_ptr, key_len, value_ptr, value_len, fun, line);
			break;
		default:
				ret = dave_false;
			break;
	}

	return ret;
}

ub
ramkv_local_inq(KV *pKV, sb index, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	ub ret;

	KVDEBUG("name:%s attrib:%d", pKV->name, pKV->attrib);

	switch(pKV->attrib)
	{
		case KvAttrib_ram:
				ret = ramkv_local_multimap_inq(pKV, index, key_ptr, key_len, value_ptr, value_len);
			break;
		case KvAttrib_list:
				ret = __ramkv_list_inq__(pKV, index, key_ptr, key_len, value_ptr, value_len, fun, line);
			break;
		default:
				ret = 0;
			break;
	}

	return ret;
}

ub
ramkv_local_del(KV *pKV, u8 *key_ptr, ub key_len, void *value_ptr, ub value_len, s8 *fun, ub line)
{
	ub ret;

	KVDEBUG("name:%s attrib:%d %d <%s:%d>",
		pKV->name, pKV->attrib, key_len,
		fun, line);

	switch(pKV->attrib)
	{
		case KvAttrib_ram:
				ret = ramkv_local_multimap_del(pKV, key_ptr, key_len, value_ptr, value_len);
			break;
		case KvAttrib_list:
				ret =  __ramkv_list_del__(pKV, key_ptr, key_len, value_ptr, value_len, fun, line);
			break;
		default:
				ret = 0;
			break;
	}

	return ret;
}

dave_bool
ramkv_local_top(KV *pKV, u8 *key_ptr, ub key_len)
{
	dave_bool ret;

	switch(pKV->attrib)
	{
		case KvAttrib_ram:
				/*
				 * 不方便计算top。
				 */
				KVLOG("Top calculation is not supported on attrib:%d", pKV->attrib);
				ret = dave_false;
			break;
		case KvAttrib_list:
				ret =  __ramkv_list_top__(pKV, key_ptr, key_len);
			break;
		default:
				KVLOG("unsupport attrib:%d", pKV->attrib);
				ret = dave_false;
			break;
	}

	return ret;
}

#endif


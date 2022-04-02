/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "thread_tools.h"
#include "thread_log.h"

static void *_pKV = NULL;

// =====================================================================

void
thread_remote_id_table_init(void)
{
	_pKV = base_kv_malloc((s8 *)"threadremoteidtable", KVAttrib_ram, 0, NULL);
}

void
thread_remote_id_table_exit(void)
{
	if(_pKV != NULL)
	{
		base_kv_free(_pKV, NULL);
		_pKV = NULL;
	}
}

void
thread_remote_id_table_add(ThreadId remote_id, s8 *remote_name)
{
	THREADDEBUG("%s/%lx", remote_name, remote_id);

	remote_id = thread_clean_wakeup(remote_id);

	if((thread_is_remote(remote_id) == dave_true) && (_pKV != NULL))
	{
		base_kv_add_ub_ptr(_pKV, (ub)(remote_id), _pKV);
	}
}

void
thread_remote_id_table_del(ThreadId remote_id, s8 *remote_name)
{
	THREADDEBUG("%s/%lx", remote_name, remote_id);

	remote_id = thread_clean_wakeup(remote_id);

	if((thread_is_remote(remote_id) == dave_true) && (_pKV != NULL))
	{
		if(base_kv_inq_ub_ptr(_pKV, (ub)remote_id) != NULL)
		{
			base_kv_del_ub_ptr(_pKV, (ub)(remote_id));
		}
	}
}

dave_bool
thread_remote_id_table_inq(ThreadId remote_id)
{
	dave_bool ret;

	if((_pKV == NULL)
		|| (thread_is_sync(remote_id) == dave_true)
		|| (thread_attrib(remote_id) != REMOTE_TASK_ATTRIB)
		|| (thread_is_remote(remote_id) == dave_false)
		|| (thread_get_thread(remote_id) == 0xffff)
		|| (thread_get_net(remote_id) == 0xffff))
	{
		return dave_true;
	}

	remote_id = thread_clean_wakeup(remote_id);

	if(base_kv_inq_ub_ptr(_pKV, (ub)remote_id) == NULL)
	{
		ret = dave_false;
	}
	else
	{
		ret = dave_true;
	}

	THREADDEBUG("%lx/%s", thread_id, ret==dave_true?"true":"false");

	return ret;
}

#endif


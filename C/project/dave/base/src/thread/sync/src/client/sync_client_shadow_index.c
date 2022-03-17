/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_CLIENT)
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "thread_tools.h"
#include "base_rxtx.h"
#include "sync_base_package.h"
#include "sync_param.h"
#include "sync_cfg.h"
#include "sync_client_tools.h"
#include "sync_client_data.h"
#include "sync_client_tx.h"
#include "sync_test.h"
#include "sync_lock.h"
#include "sync_log.h"

static TLock _sync_client_shadow_index_pv;

static SyncServer *
_sync_client_shadow_index_find_other(SyncServer *pMyServer)
{
	SyncServer *pHeadServer = sync_client_data_head_server();
	SyncServer *pOtherServer = NULL;
	ub server_index;

	for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
	{
		if(((&pHeadServer[server_index]) != pMyServer) && (pHeadServer[server_index].globally_identifier[0] != '\0'))
		{
			if(dave_strcmp(pHeadServer[server_index].globally_identifier, pMyServer->globally_identifier) == dave_true)
			{
				if(pOtherServer == NULL)
				{
					pOtherServer = &pHeadServer[server_index];
				}
				else
				{
					SYNCABNOR("A systemic error has occurred. This system has two identical global identifiers. %lx/%s %lx/%s",
						pOtherServer, pOtherServer->verno,
						&pHeadServer[server_index], pHeadServer[server_index].verno);
				}
			}
		}
	}

	return pOtherServer;
}

static dave_bool
_sync_client_shadow_the_server_be_alone(SyncServer *pServer, LinkThread *pThread)
{
	LinkThread *pSearchThread = sync_client_data_head_thread();
	ub thread_index, server_index;

	for(thread_index=0; thread_index<SYNC_THREAD_MAX; thread_index++)
	{
		if(pThread != &pSearchThread[thread_index])
		{
			for(server_index=0; server_index<SERVER_DATA_MAX; server_index++)
			{
				if(pSearchThread[thread_index].pServer[server_index] == pServer)
					return dave_false;
			}
		}
	}

	return dave_true;
}

static void
__sync_client_shadow_index_add(SyncServer *pMyServer, SyncServer *pOtherServer)
{
	if(pOtherServer == NULL)
	{
		/*
		 * 这个时候没有任何影子索引被确定，直接确定自己的索引为影子索引。
		 */
		pMyServer->shadow_index = pMyServer->server_index;
	}
	else
	{
		if(pOtherServer->shadow_index >= SERVER_DATA_MAX)
		{
			pOtherServer->shadow_index = pMyServer->shadow_index = pMyServer->server_index;
		}
		else
		{
			pMyServer->shadow_index = pOtherServer->shadow_index;
		}
	}
}

static void
__sync_client_shadow_index_del(SyncServer *pMyServer, SyncServer *pOtherServer, LinkThread *pThread)
{
	if(pMyServer->shadow_index == pMyServer->server_index)
	{
		if(_sync_client_shadow_the_server_be_alone(pMyServer, pThread) == dave_true)
		{
			/*
			 * 当最后一个本服务上的线程要被删除的时候，
			 * 才会清空影子索引。
			 */
			if(pOtherServer != NULL)
			{
				pOtherServer->shadow_index = SERVER_DATA_MAX;
			}

			pMyServer->shadow_index = SERVER_DATA_MAX;
		}
	}
}

static dave_bool
_sync_client_shadow_index_add(SyncServer *pMyServer, LinkThread *pThread)
{
	SyncServer *pOtherServer = _sync_client_shadow_index_find_other(pMyServer);
	dave_bool ret = dave_false;

	if(pMyServer->shadow_index >= SERVER_DATA_MAX)
	{
		__sync_client_shadow_index_add(pMyServer, pOtherServer);
	}

	if(pOtherServer != NULL)
	{
		if((pMyServer->shadow_index < SERVER_DATA_MAX)
			&& (pOtherServer->shadow_index < SERVER_DATA_MAX)
			&& (pMyServer->shadow_index != pOtherServer->shadow_index))
		{
			SYNCABNOR("Arithmetic_error. Inconsistent index found! %lx/%d/%s %lx/%d/%s",
				pMyServer, pMyServer->shadow_index, pMyServer->verno,
				pOtherServer, pOtherServer->shadow_index, pOtherServer->verno);

			pMyServer->shadow_index = pOtherServer->shadow_index = pMyServer->server_index;
		}
	}

	if(pMyServer->shadow_index == pMyServer->server_index)
	{
		ret = dave_true;
	}

	if((++ pThread->shadow_index_ready_remove_counter[pMyServer->shadow_index]) > 2)
	{
		SYNCTRACE("An anomaly is found, and it can only be added twice through two links at most! %d %d %s/%s %s ret:%d",
			pThread->shadow_index_ready_remove_flag[pMyServer->shadow_index],
			pThread->shadow_index_ready_remove_counter[pMyServer->shadow_index],
			pMyServer->globally_identifier, pMyServer->verno,
			pThread->thread_name,
			ret);

		pThread->shadow_index_ready_remove_flag[pMyServer->shadow_index] = dave_false;
		pThread->shadow_index_ready_remove_counter[pMyServer->shadow_index] = 0;
		ret = dave_true;
	}

	return ret;
}

static dave_bool
_sync_client_shadow_index_del(SyncServer *pMyServer, LinkThread *pThread)
{
	SyncServer *pOtherServer = _sync_client_shadow_index_find_other(pMyServer);
	dave_bool ret;

	if(pMyServer->shadow_index == pMyServer->server_index)
	{
		ret = dave_true;
	}
	else
	{
		ret = dave_false;
	}

	if(pMyServer->shadow_index < SERVER_DATA_MAX)
	{
		if((-- pThread->shadow_index_ready_remove_counter[pMyServer->shadow_index]) < 0)
		{
			SYNCTRACE("An anomaly is found, and it can only be added twice through two links at most! %d %s/%s %s",
				pThread->shadow_index_ready_remove_counter[pMyServer->shadow_index],
				pMyServer->globally_identifier, pMyServer->verno,
				pThread->thread_name);			
		}

		__sync_client_shadow_index_del(pMyServer, pOtherServer, pThread);
	}

	return ret;
}

// =====================================================================

void
sync_client_shadow_index_init(void)
{
	t_lock_reset(&_sync_client_shadow_index_pv);
}

void
sync_client_shadow_index_exit(void)
{

}

dave_bool
sync_client_shadow_index_add(SyncServer *pServer, LinkThread *pThread)
{
	dave_bool ret = dave_false;

	if(pServer->server_type == SyncServerType_sync_client)
	{
		return dave_false;
	}

	if(pServer->globally_identifier[0] == '\0')
	{
		SYNCABNOR("verno:%s type:%d has empty globally identifier!",
			pServer->verno, pServer->server_type);
		return dave_false;
	}

	SAFEZONEv3( _sync_client_shadow_index_pv, ret = _sync_client_shadow_index_add(pServer, pThread); );

	return ret;
}

dave_bool
sync_client_shadow_index_del(SyncServer *pServer, LinkThread *pThread)
{
	dave_bool ret = dave_false;

	if(pServer->server_type == SyncServerType_sync_client)
	{
		return dave_false;
	}

	if(pServer->globally_identifier[0] == '\0')
	{
		SYNCABNOR("verno:%s type:%d has empty globally identifier!",
			pServer->verno, pServer->server_type);
		return dave_false;
	}

	SAFEZONEv3( _sync_client_shadow_index_pv, ret = _sync_client_shadow_index_del(pServer, pThread); );

	return ret;
}

#endif


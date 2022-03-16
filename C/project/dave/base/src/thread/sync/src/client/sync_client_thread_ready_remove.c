/*
 * ================================================================================
 * (c) Copyright 2021 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2021.09.30.
 * The synchronization service in the future may try to connect to all within
 * the framework of the server.
 * ================================================================================
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_CLIENT)
#include "dave_base.h"
#include "dave_tools.h"
#include "thread_parameter.h"
#include "thread_tools.h"
#include "thread_remote_id_table.h"
#include "thread_gid_table.h"
#include "sync_param.h"
#include "sync_client_shadow_index.h"
#include "sync_base_package.h"
#include "sync_client_data.h"
#include "sync_test.h"
#include "sync_lock.h"
#include "sync_log.h"

typedef struct {
	dave_bool ready_or_remove_flag;

	s8 thread_name[SYNC_THREAD_NAME_LEN];
	ThreadId thread_id;
	s8 globally_identifier[DAVE_GLOBALLY_IDENTIFIER_LEN];
	SyncServer *pServer;
	LinkThread *pThread;
} ThreadActive;

static TLock _thread_active_pv;
static void *_thread_active_kv = NULL;

static void _sync_client_thread_active_timer(void *kv, s8 *key);
static void _sync_client_thread_kv_empty_check(void);

static void
_sync_client_thread_ready_msg(s8 *thread_name, ThreadId thread_id, s8 *globally_identifier)
{
	ThreadRemoteIDReadyMsg *pReady = thread_reset_msg(pReady);

	SYNCTRACE("thread:%s/%lx", thread_name, thread_id);

	pReady->remote_thread_id = thread_id;
	dave_strcpy(pReady->remote_thread_name, thread_name, sizeof(pReady->remote_thread_name));
	dave_strcpy(pReady->globally_identifier, globally_identifier, sizeof(pReady->globally_identifier));

	broadcast_local(MSGID_REMOTE_THREAD_ID_READY, pReady);
}

static void
_sync_client_thread_remove_msg(s8 *thread_name, ThreadId thread_id, s8 *globally_identifier)
{
	ThreadRemoteIDRemoveMsg *pRemove = thread_reset_msg(pRemove);

	SYNCTRACE("thread:%s/%lx", thread_name, thread_id);

	pRemove->remote_thread_id = thread_id;
	dave_strcpy(pRemove->remote_thread_name, thread_name, sizeof(pRemove->remote_thread_name));
	dave_strcpy(pRemove->globally_identifier, globally_identifier, sizeof(pRemove->globally_identifier));

	broadcast_local(MSGID_REMOTE_THREAD_ID_REMOVE, pRemove);
}

static void
_sync_client_thread_active_msg(ThreadActive *pActive)
{
	if(pActive->ready_or_remove_flag == dave_true)
	{
		_sync_client_thread_ready_msg(pActive->thread_name, pActive->thread_id, pActive->globally_identifier);
	}
	else
	{
		_sync_client_thread_remove_msg(pActive->thread_name, pActive->thread_id, pActive->globally_identifier);
	}
}

static ThreadActive *
_sync_client_thread_active_malloc(dave_bool ready_or_remove_flag, ThreadId thread_id, SyncServer *pServer, LinkThread *pThread)
{
	ThreadActive *pActive = dave_malloc(sizeof(ThreadActive));

	pActive->ready_or_remove_flag = ready_or_remove_flag;

	dave_strcpy(pActive->thread_name, thread_name(thread_id), sizeof(pActive->thread_name));
	pActive->thread_id = thread_id;
	dave_strcpy(pActive->globally_identifier, pServer->globally_identifier, sizeof(pActive->globally_identifier));
	pActive->pServer = pServer;
	pActive->pThread = pThread;

	return pActive;
}

static void
_sync_client_thread_active_free(ThreadActive *pActive)
{
	if(pActive != NULL)
	{
		pActive->pServer = NULL;
		pActive->pThread = NULL;

		dave_free(pActive);
	}
}

static void
_sync_client_thread_active_push(dave_bool ready_or_remove_flag, ThreadId thread_id, SyncServer *pServer, LinkThread *pThread)
{
	s8 active_key[128];
	ThreadActive *pActive;

	dave_snprintf(active_key, sizeof(active_key), "%lx%lx", pServer, pThread);

	pActive = base_kv_inq_key_ptr(_thread_active_kv, active_key);
	if(pActive == NULL)
	{
		pActive = _sync_client_thread_active_malloc(ready_or_remove_flag, thread_id, pServer, pThread);

		base_kv_add_key_ptr(_thread_active_kv, active_key, pActive);
	}
	else
	{
		if(pActive->thread_id != thread_id)
		{
			SYNCABNOR("thread_id:%lx/%lx mismatch!", pActive->thread_id != thread_id);
			pActive->thread_id = thread_id;
		}

		if(pActive->ready_or_remove_flag == ready_or_remove_flag)
		{
			SYNCLOG("find two %s message! %s/%s",
				ready_or_remove_flag==dave_true?"ready":"remove",
				pServer->verno, pThread->thread_name);
		}
		else
		{
			/*
			 * 前一个状态没有来得急发出时，
			 * 新的状态又来了，这个时候可以直接把前一个状态删除。
			 */
			SYNCTRACE("Status change, delete reminder. verno:%s thread:%s %s/%s",
				pServer->verno, pThread->thread_name,
				pActive->ready_or_remove_flag==dave_true?"ready":"remove",
				ready_or_remove_flag==dave_true?"ready":"remove");

			_sync_client_thread_active_free(base_kv_del_key_ptr(_thread_active_kv, active_key));
		}
	}
}

static void
_sync_client_thread_active_pop(void *kv, s8 *key)
{
	ThreadActive *pActive = base_kv_inq_key_ptr(_thread_active_kv, key);
	dave_bool free_flag = dave_false;

	if(pActive == NULL)
	{
		SYNCABNOR("Arithmetic error!");
		base_kv_del_key_ptr(_thread_active_kv, key);
	}
	else
	{
		SYNCTRACE("server_socket:%d server_ready:%d ready_or_remove_flag:%d thread:%s/%lx",
			pActive->pServer->server_socket, pActive->pServer->server_ready,
			pActive->ready_or_remove_flag,
			pActive->thread_name, pActive->thread_id);

		if(pActive->pServer->server_socket == INVALID_SOCKET_ID)
		{
			free_flag = dave_true;
			if(pActive->ready_or_remove_flag == dave_false)
			{
				_sync_client_thread_active_msg(pActive);
			}
		}
		else
		{
			if((pActive->ready_or_remove_flag == dave_false)
				|| (pActive->pServer->server_ready == dave_true))
			{
				free_flag = dave_true;
				_sync_client_thread_active_msg(pActive);
			}
		}

		if(free_flag == dave_true)
		{
			_sync_client_thread_active_free(base_kv_del_key_ptr(_thread_active_kv, key));
		}
	}
}

static void
_sync_client_thread_ready(SyncServer *pServer, LinkThread *pThread)
{
	ThreadId thread_id, shadow_id;
	dave_bool ret;

	thread_id = thread_set_remote(0, get_thread_id(pThread->thread_name), pThread->thread_index, pServer->server_index);
	thread_remote_id_table_add(thread_id, pThread->thread_name);

	ret = sync_client_shadow_index_add(pServer, pThread);
	SYNCTRACE("type:%d thread:%s ret:%d", pServer->server_type, pThread->thread_name, ret);

	if(ret == dave_true)
	{
		shadow_id = thread_set_remote(0, get_thread_id(pThread->thread_name), pThread->thread_index, pServer->shadow_index);
		thread_remote_id_table_add(shadow_id, pThread->thread_name);
		thread_gid_table_add(pServer->globally_identifier, pThread->thread_name, shadow_id);

		if(pThread->shadow_index_ready_remove_flag[pServer->shadow_index] == dave_false)
		{
			SYNCTRACE("type:%d thread:%s", pServer->server_type, pThread->thread_name);

			pThread->shadow_index_ready_remove_flag[pServer->shadow_index] = dave_true;
			_sync_client_thread_active_push(pThread->shadow_index_ready_remove_flag[pServer->shadow_index], shadow_id, pServer, pThread);
		}
	}
}

static void
_sync_client_thread_remove(SyncServer *pServer, LinkThread *pThread)
{
	ub backup_shadow_index = pServer->shadow_index;
	ThreadId thread_id, shadow_id;
	dave_bool ret;

	thread_id = thread_set_remote(0, get_thread_id(pThread->thread_name), pThread->thread_index, pServer->server_index);
	thread_remote_id_table_del(thread_id, pThread->thread_name);

	ret = sync_client_shadow_index_del(pServer, pThread);
	SYNCTRACE("type:%d thread:%s ret:%d", pServer->server_type, pThread->thread_name, ret);

	if(ret == dave_true)
	{
		if(backup_shadow_index >= SERVER_DATA_MAX)
		{
			SYNCTRACE("Find invalid shadow_index:%d! If you are in a shutdown state, there is no problem.",
				backup_shadow_index);
		}
		else
		{
			shadow_id = thread_set_remote(0, get_thread_id(pThread->thread_name), pThread->thread_index, backup_shadow_index);
			thread_remote_id_table_del(shadow_id, pThread->thread_name);
			thread_gid_table_del(pServer->globally_identifier, pThread->thread_name);

			if(pThread->shadow_index_ready_remove_flag[backup_shadow_index] == dave_true)
			{
				SYNCTRACE("type:%d thread:%s", pServer->server_type, pThread->thread_name);

				pThread->shadow_index_ready_remove_flag[backup_shadow_index] = dave_false;
				_sync_client_thread_active_push(pThread->shadow_index_ready_remove_flag[backup_shadow_index], shadow_id, pServer, pThread);
			}
		}
	}
}

static ErrCode
_sync_client_thread_kv_recycle(void *kv, s8 *key)
{
	ThreadActive *pActive = base_kv_del_key_ptr(kv, key);

	if(pActive == NULL)
		return ERRCODE_empty_data;

	_sync_client_thread_active_free(pActive);

	return ERRCODE_OK;
}

static void
_sync_client_thread_active_timer_malloc(void)
{
	if(_thread_active_kv == NULL)
	{
		SYNCTRACE("");
		_thread_active_kv = base_kv_malloc((s8 *)"threadactive", KVAttrib_list, 1, _sync_client_thread_active_timer);
	}
}

static void
_sync_client_thread_active_timer_free(void)
{
	if(_thread_active_kv != NULL)
	{
		base_kv_free(_thread_active_kv, _sync_client_thread_kv_recycle);
		_thread_active_kv = NULL;
	}
}

static void
_sync_client_thread_kv_empty_check(void)
{
	if(_thread_active_kv != NULL)
	{
		if(base_kv_inq_top_ptr(_thread_active_kv) == NULL)
		{
			_sync_client_thread_active_timer_free();
		}
	}
}

static void
_sync_client_thread_temporarily_define_message(MSGBODY *thread_msg)
{
	SAFEZONEv3(_thread_active_pv, _sync_client_thread_kv_empty_check(););
}

static void
_sync_client_thread_active_timer(void *kv, s8 *key)
{
	SAFEZONEidlev3(_thread_active_pv, {

		if(_thread_active_kv != NULL)
		{
			if(base_power_state() == dave_true)
			{
				_sync_client_thread_active_pop(kv, key);
			}
		}

	} );

	TemporarilyDefineMessageMsg *pTemp = thread_msg(pTemp);
	write_msg(self(), MSGID_TEMPORARILY_DEFINE_MESSAGE, pTemp);
}

// =====================================================================

void
sync_client_thread_ready_remove_init(void)
{
	sync_client_shadow_index_init();

	t_lock_reset(&_thread_active_pv);

	reg_msg(MSGID_TEMPORARILY_DEFINE_MESSAGE, _sync_client_thread_temporarily_define_message);
}

void
sync_client_thread_ready_remove_exit(void)
{
	SAFEZONEv3(_thread_active_pv, _sync_client_thread_active_timer_free(););

	unreg_msg(MSGID_TEMPORARILY_DEFINE_MESSAGE);

	sync_client_shadow_index_exit();
}

void
sync_client_thread_ready(SyncServer *pServer, LinkThread *pThread)
{
	if((pServer->server_type == SyncServerType_sync_client)
		|| (pThread->thread_name[0] == '\0'))
	{
		return;
	}

	SYNCTRACE("%s %s", pServer->globally_identifier, pThread->thread_name);

	SAFEZONEv3(_thread_active_pv, {

		_sync_client_thread_active_timer_malloc();
		_sync_client_thread_ready(pServer, pThread);
		_sync_client_thread_kv_empty_check();

	} );
}

void
sync_client_thread_remove(SyncServer *pServer, LinkThread *pThread)
{
	if(pServer->server_type == SyncServerType_sync_client)
	{
		return;
	}

	SYNCTRACE("%s(%d/%d) %s ready_remove_flag:%d/%d counter:%d/%d",
		pServer->globally_identifier, pServer->shadow_index, pServer->server_index,
		pThread->thread_name,
		pThread->shadow_index_ready_remove_flag[pServer->shadow_index],
		pThread->shadow_index_ready_remove_flag[pServer->server_index],
		pThread->shadow_index_ready_remove_counter[pServer->shadow_index],
		pThread->shadow_index_ready_remove_counter[pServer->server_index]);

	SAFEZONEv3(_thread_active_pv, {

		_sync_client_thread_active_timer_malloc();
		_sync_client_thread_remove(pServer, pThread);
		_sync_client_thread_kv_empty_check();

	} );
}

#endif

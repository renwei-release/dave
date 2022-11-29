/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_CLIENT)
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "thread_parameter.h"
#include "thread_tools.h"
#include "sync_param.h"
#include "sync_client_data.h"
#include "sync_client_load_balancer.h"
#include "sync_client_route.h"
#include "sync_test.h"
#include "sync_lock.h"
#include "sync_log.h"

#define SYNC_CLIENT_THREAD_MAX 1

static ThreadId _sync_client_thread = INVALID_THREAD_ID;

static ub
_sync_client_thread_number(void)
{
	ub thread_number = dave_os_cpu_process_number();

	if(thread_number > SYNC_CLIENT_THREAD_MAX)
	{
		thread_number = SYNC_CLIENT_THREAD_MAX;
	}

	return thread_number;
}

static inline dave_bool
_sync_client_remote_forwarded_msg(ub msg_id)
{
	dave_bool ret;

	switch((ub)msg_id)
	{
		case MSGID_RESERVED:
		case MSGID_TEST:
		case MSGID_TIMER:
		case MSGID_WAKEUP:
		case MSGID_RUN_FUNCTION:
		case MSGID_RESTART_REQ:
		case MSGID_RESTART_RSP:
		case MSGID_POWER_OFF:
		case MSGID_REMOTE_THREAD_READY:
		case MSGID_REMOTE_THREAD_REMOVE:
		case MSGID_TRACE_SWITCH:
		case MSGID_PROCESS_MSG_TIMER_OUT:
		case MSGID_SYSTEM_MOUNT:
		case MSGID_SYSTEM_DECOUPLING:
		case MSGID_MEMORY_WARNING:
		case MSGID_THREAD_BUSY:
		case MSGID_THREAD_IDLE:
		case MSGID_CFG_UPDATE:
		case MSGID_INVALID:
				ret = dave_false;
			break;
		default:
				ret = dave_true;
			break;
	}

	return ret;
}

static inline dave_bool
_sync_client_can_be_route(MSGBODY *pMsg)
{
	if(pMsg->msg_type == BaseMsgType_Broadcast_local)
	{
		return dave_false;
	}

	return dave_true;
}

static inline void
_sync_client_message_route(MSGBODY *pMsg)
{
	if(_sync_client_can_be_route(pMsg) == dave_false)
	{
		SYNCLTRACE(360,1,"%s->%s:%d %d can't be route!",
			thread_name(pMsg->msg_src),
			thread_name(pMsg->msg_dst),
			pMsg->msg_id,
			pMsg->msg_type);
		return;
	}

	SYNCDEBUG("%lx/%s->%lx/%s:%s",
		pMsg->msg_src, thread_name(pMsg->msg_src),
		pMsg->msg_dst, thread_name(pMsg->msg_dst),
		msgstr(pMsg->msg_id));

	sync_client_message_route(pMsg);
}

static void
_sync_client_remote_init(MSGBODY *pMsg)
{

}

static void
_sync_client_remote_main(MSGBODY *pMsg)
{
	if(_sync_client_remote_forwarded_msg(pMsg->msg_id) == dave_true)
	{
		_sync_client_message_route(pMsg);
	}
}

static void
_sync_client_remote_exit(MSGBODY *pMsg)
{

}

static ThreadId
_sync_client_thread_creat(s8 *name)
{
	ub thread_number = _sync_client_thread_number();
	ThreadId remote_id;

	remote_id = base_thread_creat((char *)name, thread_number, THREAD_REMOTE_FLAG|THREAD_THREAD_FLAG, _sync_client_remote_init, _sync_client_remote_main, _sync_client_remote_exit);

	if(remote_id >= SYNC_THREAD_MAX)
	{
		SYNCABNOR("remote_id<%d/%d> is out of range!", remote_id, SYNC_THREAD_MAX);
	}

	return remote_id;
}

static dave_bool
_sync_client_thread_del(ThreadId remote_id)
{
	dave_bool ret;

	ret = base_thread_del(remote_id);

	if(ret == dave_true)
	{
		if(remote_id >= SYNC_THREAD_MAX)
		{
			SYNCABNOR("remote_id<%d/%d> is out of range!", remote_id, SYNC_THREAD_MAX);
		}
	}

	return ret;
}

// =====================================================================

void
sync_client_thread_init(void)
{
	_sync_client_thread = thread_id(SYNC_CLIENT_THREAD_NAME);
}

void
sync_client_thread_exit(void)
{

}

ThreadId
sync_client_thread_add(SyncServer *pServer, s8 *thread_name)
{
	ThreadId remote_thread_id = INVALID_THREAD_ID;

	SYNCDEBUG("pServer:%x thread_name:%s", pServer, thread_name);

	if(thread_name[0] != '\0')
	{
		remote_thread_id = thread_id(thread_name);
		if(remote_thread_id == INVALID_THREAD_ID)
		{
			remote_thread_id = _sync_client_thread_creat(thread_name);

			if(remote_thread_id == INVALID_THREAD_ID)
			{
				SYNCABNOR("creat remote thread:%s failed!", thread_name);
			}
			else
			{
				SYNCTRACE("creat remote thread:%s success!", thread_name);
			}
		}
		else
		{
			SYNCTRACE("this thread<%s> already exists locally.", thread_name);
		}
	}
	else
	{
		SYNCABNOR("get empty remote thread name!");
	}

	return remote_thread_id;
}

dave_bool
sync_client_thread_del(SyncServer *pServer, s8 *thread_name)
{
	ThreadId remote_thread_id;
	TaskAttribute attrib;

	SYNCTRACE("pServer:%s/%d thread_name:%s",
		pServer->verno, pServer->server_type,
		thread_name);

	if((thread_name != NULL) && (thread_name[0] != '\0'))
	{
		remote_thread_id = thread_id(thread_name);
		if(remote_thread_id != INVALID_THREAD_ID)
		{
			attrib = thread_attrib(remote_thread_id);
			if(attrib == REMOTE_TASK_ATTRIB)
			{
				if(_sync_client_thread_del(remote_thread_id) == dave_true)
				{
					SYNCTRACE("del remote thread:%s success!", thread_name);
				}
				else
				{
					SYNCLOG("del remote thread:%s failed!", thread_name);
				}
			}
			else
			{
				SYNCTRACE("the thread:%s<%d> is not a remote thread!", thread_name, attrib);
			}
		}
		else
		{
			SYNCTRACE("the thread:%s not here!", thread_name);
		}
	}
	else
	{
		SYNCLOG("get empty remote thread name:%s!", thread_name);
	}

	return dave_true;
}

#endif


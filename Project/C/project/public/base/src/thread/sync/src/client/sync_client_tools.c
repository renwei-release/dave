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
#include "sync_base_package.h"
#include "sync_param.h"
#include "sync_client_tools.h"
#include "sync_client_data.h"
#include "sync_test.h"
#include "sync_lock.h"
#include "sync_log.h"

// =====================================================================

void
sync_client_test_data(SyncTestMsg *pTestMsg)
{
	ub str_index;

	dave_memset(pTestMsg, 0x00, sizeof(pTestMsg));

	pTestMsg->u8_test_data = 0x15;

	pTestMsg->u16_test_data = 0x5a55;

	pTestMsg->u32_test_data = 0x1234ef5a;

	pTestMsg->ub_test_data = 0x907655aa;

	pTestMsg->double_test_data = 12765.123;

	for(str_index=0; str_index<64; str_index++)
	{
		pTestMsg->str_test_data[str_index] = '0' + str_index;
	}

	pTestMsg->ptr = (void *)(65242492827);
}

dave_bool
sync_client_test_data_valid(SyncTestMsg *pTestMsg)
{
	SyncTestMsg true_test_data;
	ub str_index;

	if(pTestMsg == NULL)
	{
		SYNCABNOR("pTestMsg is NULL!");
		return dave_false;
	}

	sync_client_test_data(&true_test_data);

	if(true_test_data.u8_test_data != pTestMsg->u8_test_data)
	{
		SYNCABNOR("u8 data mismatch!%x,%x", true_test_data.u8_test_data, pTestMsg->u8_test_data);
		return dave_false;
	}

	if(true_test_data.u16_test_data != pTestMsg->u16_test_data)
	{
		SYNCABNOR("u16 data mismatch!%x,%x", true_test_data.u16_test_data, pTestMsg->u16_test_data);
		return dave_false;
	}

	if(true_test_data.u32_test_data != pTestMsg->u32_test_data)
	{
		SYNCABNOR("u32 data mismatch!%x,%x", true_test_data.u32_test_data, pTestMsg->u32_test_data);
		return dave_false;
	}

	if(true_test_data.ub_test_data != pTestMsg->ub_test_data)
	{
		SYNCABNOR("ub data mismatch!%x,%x", true_test_data.ub_test_data, pTestMsg->ub_test_data);
		return dave_false;
	}

	if(true_test_data.double_test_data != pTestMsg->double_test_data)
	{
		SYNCABNOR("double data mismatch!%lf,%lf", true_test_data.double_test_data, pTestMsg->double_test_data);
		return dave_false;
	}

	for(str_index=0; str_index<64; str_index++)
	{
		if(true_test_data.str_test_data[str_index] != pTestMsg->str_test_data[str_index])
		{
			SYNCABNOR("str data mismatch!%x,%x,%d",
				true_test_data.str_test_data[str_index], pTestMsg->str_test_data[str_index], str_index);
			return dave_false;
		}
	}

	if(true_test_data.ptr != pTestMsg->ptr)
	{
		SYNCABNOR("ptr data mismatch!%x,%x", true_test_data.ptr, pTestMsg->ptr);
		return dave_false;
	}

	return dave_true;
}

s8 *
sync_client_type_to_str(SyncServerType type)
{
	s8 *type_str;

	switch(type)
	{
		case SyncServerType_sync_client:
				type_str = (s8 *)"sync  ";
			break;
		case SyncServerType_child:
				type_str = (s8 *)"child ";
			break;
		case SyncServerType_client:
				type_str = (s8 *)"client";
			break;
		default:
				type_str = (s8 *)"max";
			break;
	}

	return type_str;
}

ub
sync_client_data_thread_name_to_index(s8 *thread_name)
{
	ub thread_index, name_index;

	thread_index = 0;

	for(name_index=0; name_index<SYNC_THREAD_NAME_LEN; name_index++)
	{
		if(thread_name[name_index] == '\0')
		{
			break;
		}

		thread_index *= 0xff;

		thread_index += (ub)(thread_name[name_index]);
	}

	thread_index = thread_index % SYNC_THREAD_MAX;

	return thread_index;
}

void
__sync_client_detected_rpc_efficiency__(ub msg_len, ub package_len, ub msg_id, s8 *fun, ub line)
{
	if((package_len > msg_len) && ((package_len - msg_len) > SYNC_RPC_EFFICIENCY_WARNING))
	{
		/*
		 * 这里是为了判断RPC之后的包的长度（package_len）不要过大，
		 * 不要比消息结构体（msg_len）大太多（SYNC_RPC_EFFICIENCY_WARNING）
		 * 因为目前还无法设别一些带MBUF类似结构的消息结构体，
		 * 如果不能识别，这个判断会在收到这种消息时出错。
		 * 可以考虑在RPC Python工程里面收集所有带MBUF的消息ID。
		 * 
		 */
		SYNCDEBUG("This message(%d)(P:%d M:%d) is not suitable for compression using RPC! <%s:%d>",
			msg_id, package_len, msg_len, fun, line);
	}
	if(package_len >= SYNC_RPC_BIG_PACKAGE)
	{
		SYNCLTRACE(60,1, "the msg content(%d) is too big of msg_id:%d! <%s:%d>",
			package_len, msg_id, fun, line);
	}
}

void
sync_client_send_statistics(SyncServer *pServer, s8 *thread)
{
	LinkThread *pThread = sync_client_data_thread_on_name(thread, SYNC_THREAD_INDEX_MAX);

	if(pServer != NULL)
	{
		sync_lock();
		pServer->server_send_message_counter ++;
		sync_unlock();
	}

	if(pThread != NULL)
	{
		sync_lock();
		pThread->thread_send_message_counter ++;
		sync_unlock();
	}
}

void
sync_client_recv_statistics(SyncServer *pServer, s8 *thread)
{
	LinkThread *pThread = sync_client_data_thread_on_name(thread, SYNC_THREAD_INDEX_MAX);

	if(pServer != NULL)
	{
		sync_lock();
		pServer->server_recv_message_counter ++;
		sync_unlock();
	}
	if(pThread != NULL)
	{
		sync_lock();
		pThread->thread_recv_message_counter ++;
		sync_unlock();
	}
}

ThreadId
__sync_client_thread_id_change_to_user__(ThreadId thread_id, ThreadId sync_id, s8 *fun, ub line)
{
	ThreadId new_id;

	if(thread_attrib(thread_id) == LOCAL_TASK_ATTRIB)
	{
		/*
		 * 这个线程来自远端，但它有一个本地的亲戚，
		 * 我们需要把实际发给远端的线程消息都通过SYNC转发，
		 * 不然，消息都会到达本地线程的消息队列。
		 */
		new_id = thread_set_local(thread_id, sync_id);

		SYNCDEBUG("%lx/%s->%lx/%s <%s:%d>", thread_id, thread_name(thread_id), new_id, thread_name(new_id), fun, line);

		return new_id;
	}

	return thread_id;
}

ThreadId
__sync_client_thread_id_change_from_user__(ThreadId thread_id, s8 *fun, ub line)
{
	LinkThread *pThread;
	ThreadId new_id;

	if((thread_is_remote(thread_id) == dave_true)
		&& (thread_attrib(thread_id) == LOCAL_TASK_ATTRIB))
	{
		pThread = sync_client_thread(thread_get_thread(thread_id));
		if((pThread == NULL) || (pThread->thread_name[0] == '\0'))
			return INVALID_THREAD_ID;

		new_id = thread_set_local(thread_id, thread_id(pThread->thread_name));

		SYNCDEBUG("%lx/%s->%lx/%s <%s:%d>", thread_id, thread_name(thread_id), new_id, thread_name(new_id), fun, line);

		return new_id;
	}

	return INVALID_THREAD_ID;
}

#endif


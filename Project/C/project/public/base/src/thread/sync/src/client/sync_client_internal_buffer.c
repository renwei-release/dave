/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "sync_client_internal_buffer.h"
#include "sync_client_tx.h"
#include "sync_log.h"

#define SYNC_CLIENT_INTERNAL_BUFFER_TIMER_NAME "scibp"

typedef struct {
	SyncServer *pServer;

	ub msg_id;
	ub msg_len;
	void *msg_body;

	void *next;
} InternalBufferList;

static InternalBufferList *_head = NULL, *_tail = NULL;
static TLock _internal_buffer_list_pv;

static InternalBufferList *
_sync_client_internal_buffer_malloc(SyncServer *pServer, ub msg_id, ub msg_len, void *msg_body)
{
	InternalBufferList *pList = dave_malloc(sizeof(InternalBufferList));

	pList->pServer = pServer;

	pList->msg_id = msg_id;
	pList->msg_len = msg_len;
	pList->msg_body = dave_malloc(msg_len);
	dave_memcpy(pList->msg_body, msg_body, msg_len);

	pList->next = NULL;

	return pList;
}

static void
_sync_client_internal_buffer_free(InternalBufferList *pList)
{
	dave_free(pList->msg_body);

	dave_memset(pList, 0x00, sizeof(InternalBufferList));

	dave_free(pList);
}

static void
_sync_client_internal_buffer_push(InternalBufferList *pList)
{
	SYNCTRACE("msg_id:%s msg_len:%d", msgstr(pList->msg_id), pList->msg_len);

	SAFECODEv1(_internal_buffer_list_pv, {

			if(_head == NULL)
			{
				_head = _tail = pList;
			}
			else
			{
				_tail->next = pList;
				_tail = pList;
			}
			
		}
	);
}

static dave_bool
_sync_client_internal_buffer_pop(void)
{
	InternalBufferList *temp;

	SAFECODEv1(_internal_buffer_list_pv, {

			while(_head != NULL)
			{
				if(_head->pServer->server_cnt == dave_false)
					break;

				if(sync_client_tx_run_internal_msg_req(_head->msg_id, _head->msg_len, _head->msg_body, dave_true) == dave_false)
					break;

				SYNCTRACE("msg_id:%s msg_len:%d", msgstr(_head->msg_id), _head->msg_len);

				temp = _head->next;

				_sync_client_internal_buffer_free(_head);

				_head = temp;
			}

			if(_head == NULL)
			{
				_tail = NULL;
			}
		}
	);

	if(_head == NULL)
		return dave_true;
	else
		return dave_false;
}

static void
_sync_client_internal_buffer_clean(void)
{
	InternalBufferList *temp;

	SAFECODEv1(_internal_buffer_list_pv, {

			while(_head != NULL)
			{
				temp = (InternalBufferList *)(_head->next);
				_sync_client_internal_buffer_free(_head);
				_head = temp;
			}

		}
	);

	_head = _tail = NULL;
}

static void
_sync_client_internal_buffer_timer_out(TIMERID timer_id, ub thread_index)
{
	sync_client_internal_buffer_pop();
}

// =====================================================================

void
sync_client_internal_buffer_init(void)
{
	_head = _tail = NULL;

	t_lock_reset(&_internal_buffer_list_pv);
}

void
sync_client_internal_buffer_exit(void)
{
	_sync_client_internal_buffer_clean();
}

void
sync_client_internal_buffer_push(SyncServer *pServer, ub msg_id, ub msg_len, void *msg_body)
{
	InternalBufferList *pList = _sync_client_internal_buffer_malloc(pServer, msg_id, msg_len, msg_body);

	_sync_client_internal_buffer_push(pList);

	base_timer_creat(SYNC_CLIENT_INTERNAL_BUFFER_TIMER_NAME, _sync_client_internal_buffer_timer_out, 1000);
}

void
sync_client_internal_buffer_pop(void)
{
	if(_sync_client_internal_buffer_pop() == dave_true)
	{
		base_timer_kill(SYNC_CLIENT_INTERNAL_BUFFER_TIMER_NAME);
	}
}

#endif


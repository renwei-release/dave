/*
 * ================================================================================
 * (c) Copyright 2019 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2019.05.20.
 * ================================================================================
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_tools.h"
#include "base_rxtx.h"
#include "rxtx_param.h"
#include "rxtx_confirm_transfer.h"
#include "rxtx_log.h"

#define CT_MAX (DAVE_SERVER_SUPPORT_SOCKET_MAX)

typedef struct {
	CTNote *head;
	CTNote *tail;

	TLock pv;
} CTList;

static CTList _ct_list[CT_MAX];
static bin_ct_fun _ct_fun = NULL;

static void
_rxtx_confirm_transfer_reset(CTList *pList)
{
	pList->head = NULL;
	pList->tail = NULL;
}

static void
_rxtx_confirm_transfer_reset_all(void)
{
	ub list_index;

	for(list_index=0; list_index<CT_MAX; list_index++)
	{
		_rxtx_confirm_transfer_reset(&_ct_list[list_index]);

		t_lock_reset(&(_ct_list[list_index].pv));
	}
}

static CTNote *
_rxtx_confirm_transfer_malloc(u8 dst_ip[4], u16 dst_port, s32 socket, ORDER_CODE order_id, MBUF *data)
{
	CTNote *pNote;

	pNote = dave_malloc(sizeof(CTNote));

	dave_memcpy(pNote->dst_ip, dst_ip, 4);
	pNote->dst_port = dst_port;
	pNote->socket = socket;
	pNote->order_id = order_id;
	pNote->data = data;

	pNote->up = NULL;

	pNote->send_times = 0;
	pNote->wait_times = 0;

	return pNote;
}

static void
_rxtx_confirm_transfer_free(CTNote *pNote)
{
	if(pNote != NULL)
	{
		if(pNote->data != NULL)
		{
			dave_free(pNote->data);
		}

		dave_free(pNote);
	}
}

static dave_bool
_rxtx_confirm_transfer_push(CTList *pList, CTNote *pNote)
{
	if(pList->head == NULL)
	{
		pList->head = pNote;

		pList->tail = NULL;
	}
	else if(pList->tail == NULL)
	{
		pList->tail = pList->head;

		pList->tail->up = pNote;

		pList->head = pNote;
	}
	else
	{
		pList->head->up = pNote;

		pList->head = pNote;
	}

	return dave_true;
}

static CTNote *
_rxtx_confirm_transfer_cur(CTList *pList)
{
	CTNote *pNote;

	if(pList->tail != NULL)
	{
		pNote = pList->tail;
	}
	else if(pList->head != NULL)
	{
		pNote = pList->head;
	}
	else
	{
		pNote = NULL;
	}

	return pNote;
}

static CTNote *
_rxtx_confirm_transfer_pop(CTList *pList)
{
	CTNote *pNote;

	if(pList->tail != NULL)
	{
		pNote = pList->tail;

		pList->tail = pList->tail->up;

		if(pList->tail == pList->head)
		{
			pList->tail = NULL;
		}
	}
	else if(pList->head != NULL)
	{
		pNote = pList->head;

		pList->head = NULL;
	}
	else
	{
		pNote = NULL;
	}

	return pNote;
}

static void
_rxtx_confirm_transfer_free_list(CTList *pList)
{
	#define SAFE_COUNTER_MAX (4096000)
	ub safe_counter;
	CTNote *pNote;

	if((pList->head == NULL) && (pList->tail == NULL))
	{
		return;
	}

	safe_counter = 0;

	do {

		pNote = _rxtx_confirm_transfer_pop(pList);
		if(pNote == NULL)
		{
			break;
		}

		RTDEBUG("pNote:%lx order_id:%x data:%lx free list!",
			pNote, pNote->order_id, pNote->data);

		_rxtx_confirm_transfer_free(pNote);

		safe_counter ++;

	} while(pNote != NULL) ;

	if(safe_counter >= SAFE_COUNTER_MAX)
	{
		RTLOG("pNote too longer:%ld!", safe_counter);
	}
}

static void
_rxtx_confirm_transfer_free_all(void)
{
	ub list_index;

	for(list_index=0; list_index<CT_MAX; list_index++)
	{
		_rxtx_confirm_transfer_free_list(&_ct_list[list_index]);
	}
}

static void
_rxtx_confirm_check_wait(void)
{
	ub list_index;
	CTList *pList;
	CTNote *pNote;
	s32 socket;
	ORDER_CODE order_id;

	for(list_index=0; list_index<CT_MAX; list_index++)
	{
		pList = &_ct_list[list_index];

		if(pList->head != NULL)
		{
			socket = INVALID_SOCKET_ID;
			order_id = ORDER_CODE_END;

			SAFEZONEv3(pList->pv, {

				pNote = _rxtx_confirm_transfer_pop(pList);
				if(pNote != NULL)
				{
					if((++ pNote->wait_times) > 2)
					{
						socket = pNote->socket;
						order_id = pNote->order_id;

						RTLOG("ip:%s order_id:%x clean!",
							ipv4str(pNote->dst_ip, pNote->dst_port),
							pNote->order_id);
					}
				}
			} );

			if(socket != INVALID_SOCKET_ID)
			{
				rxtx_confirm_transfer_pop(socket, order_id);

				rxtx_confirm_transfer_out(socket, dave_false);
			}
		}
	}
}

static void
_rxtx_confirm_check_timer(TIMERID timer_id, ub thread_index)
{
	_rxtx_confirm_check_wait();
}

// =====================================================================

void
rxtx_confirm_transfer_init(bin_ct_fun ct_fun)
{
	_rxtx_confirm_transfer_reset_all();

	_ct_fun = ct_fun;

	base_timer_creat("bsct", _rxtx_confirm_check_timer, 3000);
}

void
rxtx_confirm_transfer_exit(void)
{
	_rxtx_confirm_transfer_free_all();
}

/*
 * 该模块目前最大的问题是没有给发送的节点设定一个序号，
 * 如果没有序号，那么很有可能会造成重复接收数据。
 */
dave_bool
rxtx_confirm_transfer_push(u8 dst_ip[4], u16 dst_port, s32 socket, ORDER_CODE order_id, MBUF *data)
{
	CTList *pList;
	CTNote *pNote;

	pList = &_ct_list[socket % CT_MAX];

	SAFEZONEv3(pList->pv, {

		pNote = _rxtx_confirm_transfer_malloc(dst_ip, dst_port, socket, order_id, data);

		RTDEBUG("pNote:%lx order_id:%x data:%lx push!", pNote, pNote->order_id, pNote->data);

		if(_rxtx_confirm_transfer_push(pList, pNote) == dave_false)
		{
			RTLOG("invalid transfer push! order_id:%x", order_id);

			_rxtx_confirm_transfer_free(pNote);
		}

	} );

	return dave_true;
}

dave_bool
rxtx_confirm_transfer_pop(s32 socket, ORDER_CODE order_id)
{
	CTList *pList;
	CTNote *pNote;
	dave_bool ret = dave_false;

	pList = &_ct_list[socket % CT_MAX];

	SAFEZONEv3(pList->pv, {

		pNote = _rxtx_confirm_transfer_pop(pList);

		if(pNote == NULL)
		{
			RTLOG("pNote is NULL! order_id:%x", order_id);
			ret = dave_false;
		}
		else
		{
			if(pNote->order_id != order_id)
			{
				RTLOG("order_id:%x/%x mismatch!", pNote->order_id, order_id)
			}
			else
			{
				_rxtx_confirm_transfer_free(pNote);
			}

			ret = dave_true;
		}
	} );

	return ret;
}

ub
rxtx_confirm_transfer_out(s32 socket, dave_bool resend)
{
	ub send_times = 0;
	CTList *pList;
	CTNote *pNote = NULL;

	pList = &_ct_list[socket % CT_MAX];

	SAFEZONEv3(pList->pv, {

		pNote = _rxtx_confirm_transfer_cur(pList);

		if(pNote != NULL)
		{
			if(resend == dave_true)
			{
				RTLOG("resend package:%x", pNote->order_id);
			}
		
			if((resend == dave_true) || (pNote->send_times == 0))
			{
				if(_ct_fun(pNote->dst_ip, pNote->dst_port, pNote->socket, pNote) == dave_true)
				{
					send_times = (++ pNote->send_times);
				}
			}
		}
		else
		{
			if(resend == dave_true)
			{
				RTABNOR("pNote is NULL!");
			}
		}

	} );

	return send_times;
}

void
rxtx_confirm_transfer_clean(s32 socket)
{
	CTList *pList;

	pList = &_ct_list[socket % CT_MAX];

	SAFEZONEv3(pList->pv, { _rxtx_confirm_transfer_free_list(pList); } );
}

#endif


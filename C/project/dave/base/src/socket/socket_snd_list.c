/*
 * ================================================================================
 * (c) Copyright 2016 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_os.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "base_tools.h"
#include "socket_tools.h"
#include "socket_external.h"
#include "socket_parameters.h"
#include "socket_core.h"
#include "socket_log.h"

#define SND_LIST_DEPTH_MAX 8192

static SokcetSndList *
_socket_snd_list_malloc(IPBaseInfo *pIPInfo, MBUF *data, SOCKETINFO snd_flag)
{
	SokcetSndList *pList;

	if(data == NULL)
	{
		return NULL;
	}

	pList = dave_malloc(sizeof(SokcetSndList));
	if(pIPInfo != NULL)
	{
		pList->IPInfo = *pIPInfo;
	}
	else
	{
		dave_memset(&(pList->IPInfo), 0x00, sizeof(SokcetSndList));
	}
	pList->data = data;
	pList->snd_flag = snd_flag;
	pList->next = NULL;

	return pList;
}

static void
_socket_snd_list_free(SokcetSndList *pList)
{
	dave_mfree(pList->data);

	dave_free(pList);
}

static void
_socket_snd_list_reset(SocketCoreSndList *pList)
{
	pList->snd_token = dave_false;
	pList->snd_list_depth = 0;
	pList->snd_head = pList->snd_tail = NULL;
}

static ub
_socket_snd_list_clean(SocketCoreSndList *pList)
{
	SokcetSndList *pData;
	ub safe_counter;

	safe_counter = 0;

	while(pList->snd_head != NULL)
	{
		pData = (SokcetSndList *)(pList->snd_head->next);
		_socket_snd_list_free(pList->snd_head);
		pList->snd_head = pData;
	
		safe_counter ++;
	}
	
	pList->snd_head = pList->snd_tail = NULL;

	return safe_counter;
}

static dave_bool
_socket_snd_list_catch_snd_token(SocketCoreSndList *pList, SokcetSndList *pNewList)
{
	dave_bool snd_token = dave_false;

	if(pList->snd_token == dave_false)
	{
		pList->snd_token = dave_true;

		snd_token = dave_true;
	}
	else
	{
		snd_token = dave_false;
	}

	if(pNewList != NULL)
	{
		pList->snd_list_depth ++;

		if(pList->snd_head == NULL)
		{
			pList->snd_head = pList->snd_tail = pNewList;
		}
		else
		{
			pList->snd_tail->next = pNewList;
			pList->snd_tail = pNewList;
		}
	}

	return snd_token;
}

static void
_socket_snd_list_release_snd_token(SocketCoreSndList *pList)
{
	if(pList->snd_token == dave_false)
	{
		SOCKETABNOR("snd_list_depth:%d", pList->snd_list_depth);
	}

	pList->snd_token = dave_false;
}

static SokcetSndList *
_socket_snd_list_catch_data(SocketCoreSndList *pList)
{
	SokcetSndList *pSndData;

	pSndData = pList->snd_head;
	if(pSndData != NULL)
	{
		pList->snd_list_depth --;

		pList->snd_head = pList->snd_head->next;
		if(pList->snd_head == NULL)
		{
			pList->snd_tail = NULL;
		}
	}

	return pSndData;
}

// =====================================================================

void
socket_snd_list_reset(SocketCore *pCore)
{
	SocketCoreSndList *pList = &(pCore->snd_list);

	SAFEZONEv3(pList->opt_pv_for_snd, _socket_snd_list_reset(&(pCore->snd_list)););
}

void
socket_snd_list_init(SocketCore *pCore)
{
	SocketCoreSndList *pList = &(pCore->snd_list);
	ub clean_counter = 0;

	SAFEZONEv3(pList->opt_pv_for_snd, { clean_counter = _socket_snd_list_clean(pList); });
	
	if(clean_counter > SND_LIST_DEPTH_MAX)
	{
		SOCKETABNOR("%s socket:%d/%d os_socket:%d port:%d/%d There are a lot of data(%d) that has not been sent in time.",
			thread_name(pCore->owner),
			pCore->socket_external_index, pCore->socket_internal_index,
			pCore->os_socket,
			pCore->NetInfo.port, pCore->NetInfo.src_port,
			clean_counter);
	}
}

void
socket_snd_list_exit(SocketCore *pCore)
{
	SocketCoreSndList *pList = &(pCore->snd_list);
	ub clean_counter = 0;

	SAFEZONEv3(pList->opt_pv_for_snd, { clean_counter = _socket_snd_list_clean(pList); });

	if(clean_counter > SND_LIST_DEPTH_MAX)
	{
		SOCKETLOG("%s socket:%d/%d os_socket:%d port:%d/%d There are a lot of data(%d) that has not been sent in time.",
			thread_name(pCore->owner),
			pCore->socket_external_index, pCore->socket_internal_index,
			pCore->os_socket,
			pCore->NetInfo.port, pCore->NetInfo.src_port,
			clean_counter);
	}
}

void
socket_snd_list_clean(SocketCore *pCore)
{
	SocketCoreSndList *pList = &(pCore->snd_list);

	SAFEZONEv3(pList->opt_pv_for_snd, { _socket_snd_list_clean(pList); });
}

dave_bool
socket_snd_list_catch_snd_token(SocketCore *pCore, IPBaseInfo *pIPInfo, MBUF *data, SOCKETINFO snd_flag)
{
	SocketCoreSndList *pList = &(pCore->snd_list);
	SokcetSndList *pNewList;
	dave_bool snd_token = dave_false;

	pNewList = _socket_snd_list_malloc(pIPInfo, data, snd_flag);

	SAFEZONEv3(pList->opt_pv_for_snd, { snd_token = _socket_snd_list_catch_snd_token(pList, pNewList); });

	if(pList->snd_list_depth >= SND_LIST_DEPTH_MAX)
	{
		SOCKETTRACE("%s's socket:%d/%d has snd list too depth:%d!",
			thread_name(pCore->owner),
			pCore->socket_external_index, pCore->socket_internal_index,
			pList->snd_list_depth);
	}

	return snd_token;
}

void
socket_snd_list_release_snd_token(SocketCore *pCore)
{
	SocketCoreSndList *pList = &(pCore->snd_list);

	SAFEZONEv3(pList->opt_pv_for_snd, { _socket_snd_list_release_snd_token(pList); });
}

SokcetSndList *
socket_snd_list_catch_data(SocketCore *pCore)
{
	SocketCoreSndList *pList = &(pCore->snd_list);
	SokcetSndList *pSndData = NULL;

	SAFEZONEv3(pList->opt_pv_for_snd, { pSndData = _socket_snd_list_catch_data(pList); });

	return pSndData;
}

void
socket_snd_list_release_data(SokcetSndList *pSndData)
{
	_socket_snd_list_free(pSndData);
}

#endif


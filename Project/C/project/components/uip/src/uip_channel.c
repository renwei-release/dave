/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_database.h"
#include "uip_log.h"

#define UIP_CHANNEL_KV_TABLE (s8 *)"uctKV"

typedef struct {
	ub channel_id;
	s8 channel_name[DAVE_NORMAL_NAME_LEN];
	s8 auth_key_str[DAVE_AUTH_KEY_STR_LEN];
} UIPChannelTable;

static void _uip_channel_inq_db_req(ub channel_id, void *ptr);

static ub _uip_channel_sync_table_id = 0;
static ub _uip_channel_sync_number = 0;
static void *pKV = NULL;

static dave_bool
_uip_channel_add(DBSysInqChannelRsp *pRsp)
{
	UIPChannelTable *pTable;

	pTable = base_ramkv_inq_key_ptr(pKV, pRsp->channel_name);
	if(pTable == NULL)
	{
		pTable = dave_malloc(sizeof(UIPChannelTable));
	}

	pTable->channel_id = pRsp->table_id;
	dave_strcpy(pTable->channel_name, pRsp->channel_name, DAVE_NORMAL_NAME_LEN);
	dave_strcpy(pTable->auth_key_str, pRsp->auth_key_str, DAVE_AUTH_KEY_STR_LEN);

	UIPTRACE("pTable:%x channel:%s key:%s", pTable, pTable->channel_name, pTable->auth_key_str);

	return base_ramkv_add_key_ptr(pKV, pTable->channel_name, pTable);
}

static UIPChannelTable *
_uip_channel_inq(s8 *channel_name)
{
	return (UIPChannelTable *)base_ramkv_inq_key_ptr(pKV, channel_name);
}

static RetCode
_uip_channel_del(void *kv, s8 *channel_name)
{
	UIPChannelTable *pTable;

	pTable = base_ramkv_inq_key_ptr(pKV, channel_name);
	if(pTable == NULL)
	{
		return RetCode_empty_data;
	}

	UIPTRACE("channel_name:%s", pTable->channel_name);

	base_ramkv_del_key_ptr(pKV, pTable->channel_name);

	dave_free(pTable);

	return RetCode_OK;
}

static void
_uip_channel_inq_db_rsp(MSGBODY *msg)
{
	DBSysInqChannelRsp *pRsp = (DBSysInqChannelRsp *)(msg->msg_body);

	if(pRsp->ret == ERRCODE_OK)
	{
		if(pRsp->ptr == NULL)
		{
			// sync channel.
			if(pRsp->valid_flag == dave_true)
			{
				if(_uip_channel_add(pRsp) == dave_true)
				{
					UIPTRACE("channel:%s key:%s", pRsp->channel_name, pRsp->auth_key_str);
			
					_uip_channel_sync_number ++;
				}
			}

			_uip_channel_sync_table_id = pRsp->table_id + 1;

			_uip_channel_inq_db_req(_uip_channel_sync_table_id, NULL);
		}
		else
		{
			// update channel.
			_uip_channel_add(pRsp);
		}
	}
	else
	{
		UIPLOG("uip channel table sync done(%d)!", _uip_channel_sync_number);
	}
}

static void
_uip_channel_inq_db_req(ub channel_id, void *ptr)
{
	DBSysInqChannelReq *pReq = thread_reset_msg(pReq);

	pReq->table_id = channel_id;
	pReq->ptr = ptr;

	name_event(DATABASE_THREAD_NAME, DBMSG_SYS_INQ_CHANNEL_REQ, pReq, DBMSG_SYS_INQ_CHANNEL_RSP, _uip_channel_inq_db_rsp);
}

static void
_uip_channel_table_booting_sync(void)
{
	UIPLOG("uip channel table sync booting ...");

	_uip_channel_sync_table_id = 1;
	_uip_channel_sync_number = 0;

	_uip_channel_inq_db_req(_uip_channel_sync_table_id, NULL);
}

// =====================================================================

void
uip_channel_init(void)
{
	pKV = base_ramkv_malloc(UIP_CHANNEL_KV_TABLE, KvAttrib_ram, 0, NULL);
}

void
uip_channel_exit(void)
{
	base_ramkv_free(pKV, _uip_channel_del);
}

void 
uip_channel_reset(void)
{
	_uip_channel_table_booting_sync();
}

RetCode
uip_channel_verify(s8 *channel, s8 *auth_key_str)
{
	UIPChannelTable *pTable;
	RetCode ret = RetCode_OK;

	if((NULL == channel) || ('\0' == channel[0]))
	{
		UIPLOG("Invalid channel!");
		return RetCode_Invalid_channel;
	}

	if((NULL == auth_key_str) || ('\0' == auth_key_str[0]))
	{
		UIPLOG("Invalid auth_key_str!");
		return RetCode_channel_not_exist;
	}

	pTable = _uip_channel_inq(channel);
	if(pTable != NULL)
	{
		if(dave_strcmp(channel, pTable->channel_name) == dave_false)
		{
			UIPLOG("channel:%s/%s mismatch!",
				channel, pTable->channel_name);
			ret = RetCode_Invalid_channel;
		}

		if(dave_strcmp(auth_key_str, pTable->auth_key_str) == dave_false)
		{
			UIPLOG("the channel:%s auth_key_str:%s/%s mismatch!",
				channel, auth_key_str, pTable->auth_key_str);
			ret = RetCode_Invalid_channel;
		}
	}
	else
	{
		UIPLOG("channel:%s key:%s not find!", channel, auth_key_str);
		ret = RetCode_channel_not_exist;
	}

	return ret;
}

s8 *
uip_channel_inq(s8 *channel)
{
	UIPChannelTable *pTable;

	pTable = _uip_channel_inq(channel);
	if(pTable == NULL)
	{
		return NULL;
	}

	return pTable->auth_key_str;
}


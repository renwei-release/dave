/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_STORE__
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "store_msg.h"
#include "store_log.h"

#define CFG_REDIS_ADDRESS "REDISADDRESS"
#define CFG_REDIS_PORT "REDISPORT"
#define CFG_REDIS_PWD "REDISPASSWORD"

typedef struct {
	s8 redis_address[DAVE_URL_LEN];
	ub redis_port;
	s8 redis_password[DAVE_PASSWORD_LEN];

	void *redis_context;

	DateStruct connect_date;
	ub work_times;
} StoreRedis;

static ub _redis_number = 0;
static StoreRedis *_pRedis;

static void
_store_redis_connect(StoreRedis *pRedis, s8 *address, ub port, s8 *pwd)
{
	dave_strcpy(pRedis->redis_address, address, sizeof(pRedis->redis_address));
	pRedis->redis_port = port;
	dave_strcpy(pRedis->redis_password, pwd, sizeof(pRedis->redis_password));

	if(pRedis->redis_context != NULL)
	{
		dave_redis_disconnect(pRedis->redis_context);
		pRedis->redis_context = NULL;
	}

	pRedis->redis_context = dave_redis_connect(pRedis->redis_address, pRedis->redis_port, pRedis->redis_password);
}

static dave_bool
_store_redis_booting(void)
{
	s8 address[128];
	ub port;
	s8 pwd[128];
	ub redis_index, boot_number;
	StoreRedis *pRedis;

	cfg_get_str(CFG_REDIS_ADDRESS, address, sizeof(address), t_gp_localhost());
	port = cfg_get_ub(CFG_REDIS_PORT, 6379);
	cfg_get_str(CFG_REDIS_PWD, pwd, sizeof(pwd), "");

	STLOG("address:%s port:%d pwd:%s", address, port, pwd);

	boot_number = 0;

	for(redis_index=0; redis_index<_redis_number; redis_index++)
	{
		pRedis = &_pRedis[redis_index];

		if(pRedis->redis_context == NULL)
		{
			_store_redis_connect(pRedis, address, port, pwd);
		}

		if(pRedis->redis_context != NULL)
		{
			pRedis->connect_date = t_time_get_date(NULL);
			boot_number ++;
		}
	}

	STLOG("%d/%d init success!", boot_number, _redis_number);

	if(boot_number >= _redis_number)
		return dave_true;
	else
		return dave_false;
}

static void
_store_redis_boot_timer_out(TIMERID timer_id, ub thread_index)
{
	if(_store_redis_booting() == dave_true)
	{
		base_timer_die(timer_id);
	}
}

static void
_store_redis_init(ub mysql_number)
{
	_redis_number = mysql_number;

	_pRedis = dave_ralloc(_redis_number * sizeof(StoreRedis));
}

static void
_store_redis_exit(void)
{
	ub redis_index;

	for(redis_index=0; redis_index<_redis_number; redis_index++)
	{
		dave_redis_disconnect(_pRedis[redis_index].redis_context);
		_pRedis[redis_index].redis_context = NULL;
		dave_memset(&_pRedis[redis_index], 0x00, sizeof(StoreRedis));
	}

	dave_free(_pRedis);

	_pRedis = NULL;
}

static RetCode
_store_redis_command(MBUF **data, s8 *msg_ptr, ub msg_len, StoreRedis *pRedis, s8 *command)
{
	ub safe_counter;
	void *pJson;

	msg_ptr[0] = '\0';

	if((command == NULL) || (dave_strlen(command) == 0))
	{
		STLOG("invalid paramerer sql_ptr:%lx", command);
		return RetCode_Invalid_parameter;
	}

	for(safe_counter=0; safe_counter<16; safe_counter++)
	{
		pJson = dave_redis_command(pRedis->redis_context, command);
		if(pJson == NULL)
		{
			STLOG("safe_counter:%d disconnect address:%s port:%d pwd:%s",
				safe_counter,
				pRedis->redis_address, pRedis->redis_port, pRedis->redis_password);

			_store_redis_connect(pRedis, pRedis->redis_address, pRedis->redis_port, pRedis->redis_password);

			if(pRedis->redis_context != NULL)
			{
				STLOG("safe_counter:%d connect to address:%s port:%d pwd:%s",
					safe_counter,
					pRedis->redis_address, pRedis->redis_port, pRedis->redis_password);
			}

			continue;		
		}

		break;
	}

	*data = dave_json_to_mbuf(pJson);

	dave_json_free(pJson);

	return RetCode_OK;
}

// =====================================================================

void
store_redis_init(ub thread_number)
{
	_store_redis_init(thread_number);

	if(_store_redis_booting() == dave_false)
	{
		base_timer_creat("sredisboot", _store_redis_boot_timer_out, 6*1000);
	}
}

void
store_redis_exit(void)
{
	_store_redis_exit();
}

void
store_redis_command(ThreadId src, ub thread_index, StoreRedisReq *pReq)
{
	if(thread_index >= _redis_number)
	{
		STABNOR("invalid thread_index:%d _redis_number:%d", thread_index, _redis_number);
		return;
	}

	StoreRedisRsp *pRsp = thread_reset_msg(pRsp);
	StoreRedis *pRedis = &_pRedis[thread_index];

	pRedis->work_times ++;

	pRsp->ret = _store_redis_command(&(pRsp->reply), pRsp->msg, sizeof(pRsp->msg), pRedis, ms8(pReq->command));
	pRsp->ptr = pReq->ptr;

	if(pRsp->ret != RetCode_OK)
	{
		STLOG("%s execute index:%d/%d command:%s ret:%s msg:%s",
			thread_name(src),
			thread_index, _redis_number,
			ms8(pReq->command),
			retstr(pRsp->ret),
			pRsp->msg);
	}

	id_msg(src, STORE_REDIS_RSP, pRsp);

	dave_mfree(pReq->command);
}

ub
store_redis_info(s8 *info_ptr, ub info_len)
{
	ub info_index, redis_index;
	StoreRedis *pRedis;

	info_index = 0;

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "REDIS INFO(%d):\n", _redis_number);

	for(redis_index=0; redis_index<_redis_number; redis_index++)
	{
		pRedis = &_pRedis[redis_index];

		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
			" %s:%d %s %ld (%s)\n",
			pRedis->redis_address, pRedis->redis_port,
			pRedis->redis_context==NULL?"disconnect":"connect",
			pRedis->work_times,
			t_a2b_date_str(&(pRedis->connect_date)));
	}

	return info_index;
}

#endif


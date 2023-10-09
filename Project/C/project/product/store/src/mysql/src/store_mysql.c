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

#define CFG_MYSQL_ADDRESS "MySqlAddress"
#define CFG_MYSQL_PORT "MySqlPort"
#define CFG_MYSQL_USER "MySqlUser"
#define CFG_MYSQL_PWD "MySqlPwd"
#define CFG_MYSQL_DBNAME "MySqDBName"

typedef struct {
	s8 address[256];
	ub port;
	s8 user[256];
	s8 pwd[256];
	s8 db[256];

	void *mysql_client;
} StoreMysql;

static ub _mysql_number = 0;
static StoreMysql *_pMysql;

static void
_store_mysql_default_action(StoreMysql *pMysql, s8 *db)
{
	SqlRet ret;
	s8 sql[256];

	dave_snprintf(sql, sizeof(sql), "CREATE DATABASE %s", db);
	ret = dave_mysql_query(pMysql->mysql_client, sql);
	STDEBUG("ret:%s data:%s", retstr(ret.ret), dave_json_to_string(ret.pJson, NULL));
	dave_mysql_free_ret(ret);

	dave_mysql_select_db(pMysql->mysql_client, db);
}

static void
_store_mysql_init(ub mysql_number, s8 *address, ub port, s8 *user, s8 *pwd, s8 *db)
{
	ub mysql_index;
	StoreMysql *pMysql;

	_mysql_number = mysql_number;

	_pMysql = dave_ralloc(_mysql_number * sizeof(StoreMysql));

	for(mysql_index=0; mysql_index<_mysql_number; mysql_index++)
	{
		pMysql = &_pMysql[mysql_index];

		dave_strcpy(pMysql->address, address, sizeof(pMysql->address));
		pMysql->port = port;
		dave_strcpy(pMysql->user, user, sizeof(pMysql->user));
		dave_strcpy(pMysql->pwd, pwd, sizeof(pMysql->pwd));
		dave_strcpy(pMysql->db, db, sizeof(pMysql->db));

		pMysql->mysql_client = dave_mysql_creat_client(address, port, user, pwd, db);
		if(pMysql->mysql_client == NULL)
		{
			pMysql->mysql_client = dave_mysql_creat_client(address, port, user, pwd, NULL);
			if(pMysql->mysql_client == NULL)
			{
				STABNOR("%s:%d %s/%s %s creat client failed!", address, port, user, pwd, db);
				break;
			}
			else
			{
				_store_mysql_default_action(pMysql, db);
			}
		}
		else
		{
			dave_mysql_select_db(pMysql->mysql_client, db);
		}
	}

	STLOG("%d/%d init success!", mysql_index, _mysql_number)
}

static void
_store_mysql_exit(void)
{
	ub mysql_index;

	for(mysql_index=0; mysql_index<_mysql_number; mysql_index++)
	{
		dave_mysql_release_client(_pMysql[mysql_index].mysql_client);
		_pMysql[mysql_index].mysql_client = NULL;
		dave_memset(&_pMysql[mysql_index], 0x00, sizeof(StoreMysql));
	}

	dave_free(_pMysql);

	_pMysql = NULL;
}

static RetCode
_store_mysql_sql(MBUF **data, s8 *msg_ptr, ub msg_len, StoreMysql *pMysql, s8 *sql_ptr, ub sql_len)
{
	SqlRet ret;
	ub safe_counter;

	msg_ptr[0] = '\0';

	if((sql_ptr == NULL) || (sql_len == 0))
	{
		STLOG("invalid paramerer sql_ptr:%lx or sql_len:%d", sql_ptr, sql_len);
		return RetCode_Invalid_parameter;
	}

	for(safe_counter=0; safe_counter<16; safe_counter++)
	{
		ret = dave_mysql_query(pMysql->mysql_client, sql_ptr);
		if(ret.ret == RetCode_connect_error)
		{
			STLOG("safe_counter:%d disconnect address:%s port:%d user:%s pwd:%s db:%s",
				safe_counter,
				pMysql->address, pMysql->port, pMysql->user, pMysql->pwd, pMysql->db);

			dave_mysql_free_ret(ret);
		
			dave_mysql_release_client(pMysql->mysql_client);

			pMysql->mysql_client = dave_mysql_creat_client(pMysql->address, pMysql->port, pMysql->user, pMysql->pwd, pMysql->db);

			if(pMysql->mysql_client != NULL)
			{
				STLOG("safe_counter:%d connect to address:%s port:%d user:%s pwd:%s db:%s",
					safe_counter,
					pMysql->address, pMysql->port, pMysql->user, pMysql->pwd, pMysql->db);
			}

			continue;		
		}

		if(ret.ret != RetCode_OK)
		{
			dave_strcpy(msg_ptr, dave_mysql_error(pMysql->mysql_client), msg_len);
		}

		break;
	}

	*data = dave_json_to_mbuf(ret.pJson);

	dave_mysql_free_ret(ret);

	return ret.ret;
}

// =====================================================================

void
store_mysql_init(ub thread_number)
{
	s8 address[128];
	ub port;
	s8 user[128];
	s8 pwd[128];
	s8 db[128];

	dave_mysql_init();

	cfg_get_str(CFG_MYSQL_ADDRESS, address, sizeof(address), "127.0.0.1");
	port = cfg_get_ub(CFG_MYSQL_PORT, 3306);
	cfg_get_str(CFG_MYSQL_USER, user, sizeof(user), "root");
	cfg_get_str(CFG_MYSQL_PWD, pwd, sizeof(pwd), "CWLtc14@#!");
	cfg_get_str(CFG_MYSQL_DBNAME, db, sizeof(db), "DAVEDB0007");

	_store_mysql_init(thread_number, address, port, user, pwd, db);
}

void
store_mysql_exit(void)
{
	_store_mysql_exit();

	dave_mysql_exit();
}

void
store_mysql_sql(ThreadId src, ub thread_index, StoreMysqlReq *pReq)
{
	if(thread_index >= _mysql_number)
	{
		STABNOR("invalid thread_index:%d _mysql_number:%d", thread_index, _mysql_number);
		return;
	}

	StoreMysqlRsp *pRsp = thread_reset_msg(pRsp);
	StoreMysql *pMysql = &_pMysql[thread_index];

	pRsp->ret = _store_mysql_sql(&(pRsp->data), pRsp->msg, sizeof(pRsp->msg), pMysql, dave_mptr(pReq->sql), dave_mlen(pReq->sql));
	pRsp->ptr = pReq->ptr;

	if((pRsp->ret != RetCode_OK)
		&& (pRsp->ret != RetCode_table_exist)
		&& (pRsp->ret != RetCode_empty_data))
	{
		STLOG("%s execute index:%d/%d sql:%s ret:%s msg:%s",
			thread_name(src),
			thread_index, _mysql_number,
			ms8(pReq->sql),
			retstr(pRsp->ret),
			pRsp->msg);
	}

	id_msg(src, STORE_MYSQL_RSP, pRsp);

	dave_mfree(pReq->sql);
}

#endif


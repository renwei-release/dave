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
}

static void
_store_mysql_init(ub mysql_number, s8 *address, ub port, s8 *user, s8 *pwd, s8 *db)
{
	ub mysql_index;

	_mysql_number = mysql_number;

	_pMysql = dave_ralloc(_mysql_number * sizeof(StoreMysql));

	for(mysql_index=0; mysql_index<_mysql_number; mysql_index++)
	{	
		_pMysql[mysql_index].mysql_client = dave_mysql_creat_client(address, port, user, pwd, db);
		if(_pMysql[mysql_index].mysql_client == NULL)
		{
			STLOG("%s:%d %s/%s %s creat client failed!", address, port, user, pwd, db);
			break;
		}

		_store_mysql_default_action(&_pMysql[mysql_index], db);
	}
}

static void
_store_mysql_exit(void)
{
	ub mysql_index;

	for(mysql_index=0; mysql_index<_mysql_number; mysql_index++)
	{
		dave_mysql_release_client(_pMysql[mysql_index].mysql_client);
	}

	dave_free(_pMysql);

	_pMysql = NULL;
}

static RetCode
_store_mysql_sql(MBUF **data, StoreMysql *pMysql, s8 *sql_ptr, ub sql_len)
{
	SqlRet ret;

	if((sql_ptr == NULL) || (sql_len == 0))
	{
		STLOG("invalid paramerer sql_ptr:%lx or sql_len:%d", sql_ptr, sql_len);
		return RetCode_Invalid_parameter;
	}

	ret = dave_mysql_query(pMysql->mysql_client, sql_ptr);

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
	cfg_get_str(CFG_MYSQL_DBNAME, db, sizeof(db), "DB0001");

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
	StoreMysqlRsp *pRsp = thread_reset_msg(pRsp);
	StoreMysql *pMysql = &_pMysql[thread_index];

	pRsp->ret = _store_mysql_sql(&(pRsp->data), pMysql, dave_mptr(pReq->sql), dave_mlen(pReq->sql));
	pRsp->ptr = pReq->ptr;

	id_msg(src, STORE_MYSQL_RSP, pRsp);

	dave_mfree(pReq->sql);
}

#endif


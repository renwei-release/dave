/*
 * Copyright (c) 2026 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_STORE__
#include <libpq-fe.h>
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "store_msg.h"
#include "pgsql_statistics.h"
#include "store_log.h"

#define CFG_PGSQL_ADDRESS "PgSqlAddress"
#define CFG_PGSQL_PORT "PgSqlPort"
#define CFG_PGSQL_USER "PgSqlUser"
#define CFG_PGSQL_PWD "PgSqlPwd"
#define CFG_PGSQL_DBNAME "PgSqlDBName"

typedef struct {
	s8 pgsql_address[256];
	ub pgsql_port;
	s8 pgsql_user[256];
	s8 pgsql_pwd[256];
	s8 pgsql_db[256];

	PGconn *pgsql_client;

	DateStruct connect_date;

	PgsqlStatistics statistics;
} StorePgsql;

static ub _pgsql_number = 0;
static ub _pgsql_boot_times = 0;
static StorePgsql *_pPgsql = NULL;

static dave_bool
_store_pgsql_is_connection_error(PGconn *conn)
{
	if((conn == NULL) || (PQstatus(conn) != CONNECTION_OK))
	{
		return dave_true;
	}

	return dave_false;
}

static PGconn *
_store_pgsql_create_client(s8 *address, ub port, s8 *user, s8 *pwd, s8 *db)
{
	PGconn *conn;
	s8 conninfo[1024];

	if(db == NULL)
	{
		db = "postgres";
	}

	dave_snprintf(conninfo, sizeof(conninfo),
		"host=%s port=%d user=%s password=%s dbname=%s connect_timeout=6",
		address, port, user, pwd, db);

	conn = PQconnectdb(conninfo);
	if(_store_pgsql_is_connection_error(conn) == dave_true)
	{
		if(conn != NULL)
		{
			STLOG("pgsql connect failed:%s", PQerrorMessage(conn));
			PQfinish(conn);
		}

		return NULL;
	}

	return conn;
}

static void
_store_pgsql_release_client(PGconn *conn)
{
	if(conn != NULL)
	{
		PQfinish(conn);
	}
}

static RetCode
_store_pgsql_query(StorePgsql *pPgsql, s8 *sql, void **ppJson, s8 *msg_ptr, ub msg_len)
{
	PGresult *res;
	ExecStatusType status;
	RetCode ret = RetCode_OK;
	char *sql_state;
	int row_num, field_num;
	int row_index, field_index;
	void *pJson;
	void *pRow;
	char *value;

	if(ppJson != NULL)
	{
		*ppJson = NULL;
	}

	if(msg_ptr != NULL)
	{
		msg_ptr[0] = '\0';
	}

	if(_store_pgsql_is_connection_error(pPgsql->pgsql_client) == dave_true)
	{
		if(msg_ptr != NULL)
		{
			dave_strcpy(msg_ptr, "pgsql disconnected", msg_len);
		}

		return RetCode_connect_error;
	}

	res = PQexec(pPgsql->pgsql_client, sql);
	if(res == NULL)
	{
		if(msg_ptr != NULL)
		{
			dave_strcpy(msg_ptr, PQerrorMessage(pPgsql->pgsql_client), msg_len);
		}

		return RetCode_connect_error;
	}

	status = PQresultStatus(res);
	if((status != PGRES_TUPLES_OK) && (status != PGRES_COMMAND_OK))
	{
		sql_state = PQresultErrorField(res, PG_DIAG_SQLSTATE);

		if((sql_state != NULL) && (dave_strcmp(sql_state, "42P07") == dave_true))
		{
			ret = RetCode_table_exist;
		}
		else if((sql_state != NULL) && (dave_strcmp(sql_state, "42P04") == dave_true))
		{
			ret = RetCode_OK;
		}
		else if(_store_pgsql_is_connection_error(pPgsql->pgsql_client) == dave_true)
		{
			ret = RetCode_connect_error;
		}
		else
		{
			ret = RetCode_db_sql_failed;
		}

		if(msg_ptr != NULL)
		{
			dave_strcpy(msg_ptr, PQresultErrorMessage(res), msg_len);
		}

		PQclear(res);
		return ret;
	}

	if(status == PGRES_TUPLES_OK)
	{
		row_num = PQntuples(res);
		field_num = PQnfields(res);

		if((row_num > 0) && (field_num > 0))
		{
			pJson = dave_json_array_malloc();

			for(row_index=0; row_index<row_num; row_index++)
			{
				pRow = dave_json_array_malloc();

				for(field_index=0; field_index<field_num; field_index++)
				{
					if(PQgetisnull(res, row_index, field_index) != 0)
					{
						dave_json_array_add_str_len(pRow, "", 0);
					}
					else
					{
						value = PQgetvalue(res, row_index, field_index);
						dave_json_array_add_str_len(pRow, value, (ub)PQgetlength(res, row_index, field_index));
					}
				}

				dave_json_array_add_object(pJson, pRow);
			}

			if(ppJson != NULL)
			{
				*ppJson = pJson;
			}
		}
	}

	PQclear(res);

	return RetCode_OK;
}

static void
_store_pgsql_default_action(StorePgsql *pPgsql, s8 *db)
{
	RetCode ret;
	void *pJson = NULL;
	s8 msg[4096];
	s8 sql[256];

	dave_snprintf(sql, sizeof(sql), "CREATE DATABASE %s", db);
	ret = _store_pgsql_query(pPgsql, sql, &pJson, msg, sizeof(msg));
	if((ret != RetCode_OK) && (ret != RetCode_table_exist))
	{
		STABNOR("create database %s failed! ret:%s msg:%s", db, retstr(ret), msg);
	}
	else
	{
		STLOG("create database %s success! ret:%s msg:%s", db, retstr(ret), msg);
	}

	if(pJson != NULL)
	{
		dave_json_free(pJson);
		pJson = NULL;
	}
}

static void
_store_pgsql_connect(StorePgsql *pPgsql, s8 *address, ub port, s8 *user, s8 *pwd, s8 *db)
{
	PGconn *fallback_client;

	dave_strcpy(pPgsql->pgsql_address, address, sizeof(pPgsql->pgsql_address));
	pPgsql->pgsql_port = port;
	dave_strcpy(pPgsql->pgsql_user, user, sizeof(pPgsql->pgsql_user));
	dave_strcpy(pPgsql->pgsql_pwd, pwd, sizeof(pPgsql->pgsql_pwd));
	dave_strcpy(pPgsql->pgsql_db, db, sizeof(pPgsql->pgsql_db));

	if(pPgsql->pgsql_client != NULL)
	{
		_store_pgsql_release_client(pPgsql->pgsql_client);
		pPgsql->pgsql_client = NULL;
	}

	pPgsql->pgsql_client = _store_pgsql_create_client(pPgsql->pgsql_address,
		pPgsql->pgsql_port,
		pPgsql->pgsql_user,
		pPgsql->pgsql_pwd,
		pPgsql->pgsql_db);

	if(pPgsql->pgsql_client == NULL)
	{
		fallback_client = _store_pgsql_create_client(pPgsql->pgsql_address,
			pPgsql->pgsql_port,
			pPgsql->pgsql_user,
			pPgsql->pgsql_pwd,
			NULL);

		if(fallback_client == NULL)
		{
			STABNOR("%s:%d %s/%s %s create client failed!",
				pPgsql->pgsql_address,
				pPgsql->pgsql_port,
				pPgsql->pgsql_user,
				pPgsql->pgsql_pwd,
				pPgsql->pgsql_db);
		}
		else
		{
			pPgsql->pgsql_client = fallback_client;
			pPgsql->connect_date = t_time_get_date(NULL);

			_store_pgsql_default_action(pPgsql, db);

			_store_pgsql_release_client(pPgsql->pgsql_client);
			pPgsql->pgsql_client = _store_pgsql_create_client(pPgsql->pgsql_address,
				pPgsql->pgsql_port,
				pPgsql->pgsql_user,
				pPgsql->pgsql_pwd,
				pPgsql->pgsql_db);
		}
	}
	else
	{
		pPgsql->connect_date = t_time_get_date(NULL);
	}

	pgsql_statistics_reset(&(pPgsql->statistics));
}

static dave_bool
_store_pgsql_booting(void)
{
	s8 address[128];
	ub port;
	s8 user[128];
	s8 pwd[128];
	s8 db[128];
	ub pgsql_index, boot_number;
	StorePgsql *pPgsql;

	cfg_get_str(CFG_PGSQL_ADDRESS, address, sizeof(address), "127.0.0.1");
	port = cfg_get_ub(CFG_PGSQL_PORT, 5432);
	cfg_get_str(CFG_PGSQL_USER, user, sizeof(user), "postgres");
	cfg_get_str(CFG_PGSQL_PWD, pwd, sizeof(pwd), "123456");
	cfg_get_str(CFG_PGSQL_DBNAME, db, sizeof(db), "postgres");

	if((dave_strlen(address) == 0)
		|| (port == 0)
		|| (dave_strlen(user) == 0)
		|| (dave_strlen(pwd) == 0)
		|| (dave_strlen(db) == 0))
	{
		STLOG("invalid pgsql config address:%s port:%d user:%s pwd:%s db:%s", address, port, user, pwd, db);
		return dave_true;
	}

	STLOG("address:%s port:%d user:%s pwd:%s db:%s", address, port, user, pwd, db);

	boot_number = 0;

	for(pgsql_index=0; pgsql_index<_pgsql_number; pgsql_index++)
	{
		pPgsql = &_pPgsql[pgsql_index];

		if(pPgsql->pgsql_client == NULL)
		{
			_store_pgsql_connect(pPgsql, address, port, user, pwd, db);
		}

		if(pPgsql->pgsql_client != NULL)
		{
			boot_number ++;
		}
	}

	STLOG("%d/%d init success!", boot_number, _pgsql_number);

	if(boot_number >= _pgsql_number)
		return dave_true;
	else
		return dave_false;
}

static void
_store_pgsql_boot_timer_out(TIMERID timer_id, ub thread_index)
{
	if((_store_pgsql_booting() == dave_true)
		|| (++ _pgsql_boot_times > 3))
	{
		base_timer_die(timer_id);
	}
}

static void
_store_pgsql_init(ub pgsql_number)
{
	_pgsql_number = pgsql_number;
	_pgsql_boot_times = 0;

	_pPgsql = dave_ralloc(_pgsql_number * sizeof(StorePgsql));
}

static void
_store_pgsql_exit(void)
{
	ub pgsql_index;

	for(pgsql_index=0; pgsql_index<_pgsql_number; pgsql_index++)
	{
		_store_pgsql_release_client(_pPgsql[pgsql_index].pgsql_client);
		_pPgsql[pgsql_index].pgsql_client = NULL;
		dave_memset(&_pPgsql[pgsql_index], 0x00, sizeof(StorePgsql));
	}

	dave_free(_pPgsql);

	_pPgsql = NULL;
}

static RetCode
_store_pgsql_sql(MBUF **data, s8 *msg_ptr, ub msg_len, StorePgsql *pPgsql, s8 *sql_ptr, ub sql_len)
{
	RetCode ret;
	ub safe_counter;
	void *pJson = NULL;

	*data = NULL;
	msg_ptr[0] = '\0';

	if((sql_ptr == NULL) || (sql_len == 0))
	{
		STLOG("invalid parameter sql_ptr:%lx or sql_len:%d", sql_ptr, sql_len);
		return RetCode_Invalid_parameter;
	}

	for(safe_counter=0; safe_counter<16; safe_counter++)
	{
		ret = _store_pgsql_query(pPgsql, sql_ptr, &pJson, msg_ptr, msg_len);
		if(ret == RetCode_connect_error)
		{
			STLOG("safe_counter:%d disconnect address:%s port:%d user:%s db:%s",
				safe_counter,
				pPgsql->pgsql_address, pPgsql->pgsql_port, pPgsql->pgsql_user, pPgsql->pgsql_db);

			_store_pgsql_release_client(pPgsql->pgsql_client);
			pPgsql->pgsql_client = _store_pgsql_create_client(
				pPgsql->pgsql_address,
				pPgsql->pgsql_port,
				pPgsql->pgsql_user,
				pPgsql->pgsql_pwd,
				pPgsql->pgsql_db);

			if(pPgsql->pgsql_client != NULL)
			{
				STLOG("safe_counter:%d connect to address:%s port:%d user:%s db:%s",
					safe_counter,
					pPgsql->pgsql_address, pPgsql->pgsql_port, pPgsql->pgsql_user, pPgsql->pgsql_db);
			}

			if(pJson != NULL)
			{
				dave_json_free(pJson);
				pJson = NULL;
			}

			continue;
		}

		break;
	}

	if(pJson != NULL)
	{
		*data = dave_json_to_mbuf(pJson);
		dave_json_free(pJson);
		pJson = NULL;
	}

	return ret;
}

// =====================================================================

void
store_pgsql_init(ub thread_number)
{
	_store_pgsql_init(thread_number);

	if(_store_pgsql_booting() == dave_false)
	{
		base_timer_creat("spgsqlboot", _store_pgsql_boot_timer_out, 6*1000);
	}
}

void
store_pgsql_exit(void)
{
	_store_pgsql_exit();
}

void
store_pgsql_sql(ThreadId src, ub thread_index, StorePgsqlReq *pReq)
{
	if(thread_index >= _pgsql_number)
	{
		STABNOR("invalid thread_index:%d _pgsql_number:%d", thread_index, _pgsql_number);
		return;
	}

	StorePgsqlRsp *pRsp = thread_reset_msg(pRsp);
	StorePgsql *pPgsql = &_pPgsql[thread_index];

	ub start_time = dave_os_time_us();
	pRsp->ret = _store_pgsql_sql(&(pRsp->data), pRsp->msg, sizeof(pRsp->msg), pPgsql, ms8(pReq->sql), mlen(pReq->sql));
	pgsql_statistics(&(pPgsql->statistics), dave_os_time_us() - start_time, src, ms8(pReq->sql));

	pRsp->ptr = pReq->ptr;

	if((pRsp->ret != RetCode_OK)
		&& (pRsp->ret != RetCode_table_exist)
		&& (pRsp->ret != RetCode_empty_data))
	{
		STLOG("%s execute index:%d/%d sql:%s ret:%s msg:%s",
			thread_name(src),
			thread_index, _pgsql_number,
			ms8(pReq->sql),
			retstr(pRsp->ret),
			pRsp->msg);
	}

	id_msg(src, STORE_PGSQL_RSP, pRsp);

	dave_mfree(pReq->sql);
}

ub
store_pgsql_info(s8 *info_ptr, ub info_len)
{
	ub info_index, pgsql_index;
	StorePgsql *pPgsql;
	s8 statistics_str[256];

	info_index = 0;

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "PGSQL INFO(%d):\n", _pgsql_number);

	for(pgsql_index=0; pgsql_index<_pgsql_number; pgsql_index++)
	{
		pPgsql = &_pPgsql[pgsql_index];

		pgsql_statistics_info(statistics_str, sizeof(statistics_str), &(pPgsql->statistics));

		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
			" %s:%d %s (%s) (%s)\n",
			pPgsql->pgsql_address, pPgsql->pgsql_port,
			pPgsql->pgsql_client == NULL ? "disconnect" : "connect",
			t_a2b_date_str(&(pPgsql->connect_date)),
			statistics_str);
	}

	return info_index;
}

#endif

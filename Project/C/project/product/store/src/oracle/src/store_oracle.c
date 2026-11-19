/*
 * Copyright (c) 2026 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_STORE__
#define DISABLE_ORACLE

#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "store_msg.h"
#include "store_log.h"
#ifndef DISABLE_ORACLE
#include <oci.h>
#include "oracle_statistics.h"

#define CFG_ORACLE_ADDRESS "OracleAddress"
#define CFG_ORACLE_PORT "OraclePort"
#define CFG_ORACLE_USER "OracleUser"
#define CFG_ORACLE_PWD "OraclePwd"
#define CFG_ORACLE_DBNAME "OracleDBName"

#define ORACLE_FETCH_VALUE_MAX (4096)

typedef struct {
	s8 oracle_address[256];
	ub oracle_port;
	s8 oracle_user[256];
	s8 oracle_pwd[256];
	s8 oracle_db[256];

	OCIEnv *env;
	OCIError *err;
	OCIServer *server;
	OCISvcCtx *svc;
	OCISession *session;

	DateStruct connect_date;

	OracleStatistics statistics;
} StoreOracle;

static ub _oracle_number = 0;
static ub _oracle_boot_times = 0;
static StoreOracle *_pOracle = NULL;

static sword
_store_oracle_error(StoreOracle *pOracle, sb4 *error_code, s8 *msg_ptr, ub msg_len)
{
	sword status;
	text errbuf[1024];
	sb4 errcode = 0;

	if(msg_ptr != NULL)
	{
		msg_ptr[0] = '\0';
	}

	if((pOracle == NULL) || (pOracle->err == NULL))
	{
		if(error_code != NULL)
		{
			*error_code = 0;
		}

		if(msg_ptr != NULL)
		{
			dave_strcpy(msg_ptr, "oracle error handle null", msg_len);
		}

		return OCI_ERROR;
	}

	status = OCIErrorGet((dvoid *)pOracle->err, (ub4)1, NULL, &errcode, errbuf, (ub4)sizeof(errbuf), OCI_HTYPE_ERROR);

	if(error_code != NULL)
	{
		*error_code = errcode;
	}

	if((msg_ptr != NULL) && (status == OCI_SUCCESS))
	{
		dave_strcpy(msg_ptr, (s8 *)errbuf, msg_len);
	}

	return status;
}

static dave_bool
_store_oracle_is_connection_error(StoreOracle *pOracle)
{
	if((pOracle == NULL)
		|| (pOracle->env == NULL)
		|| (pOracle->err == NULL)
		|| (pOracle->server == NULL)
		|| (pOracle->svc == NULL)
		|| (pOracle->session == NULL))
	{
		return dave_true;
	}

	return dave_false;
}

static void
_store_oracle_release_client(StoreOracle *pOracle)
{
	if(pOracle == NULL)
	{
		return;
	}

	if((pOracle->svc != NULL) && (pOracle->session != NULL) && (pOracle->err != NULL))
	{
		OCISessionEnd(pOracle->svc, pOracle->err, pOracle->session, OCI_DEFAULT);
	}

	if((pOracle->server != NULL) && (pOracle->err != NULL))
	{
		OCIServerDetach(pOracle->server, pOracle->err, OCI_DEFAULT);
	}

	if(pOracle->session != NULL)
	{
		OCIHandleFree((dvoid *)pOracle->session, OCI_HTYPE_SESSION);
		pOracle->session = NULL;
	}

	if(pOracle->svc != NULL)
	{
		OCIHandleFree((dvoid *)pOracle->svc, OCI_HTYPE_SVCCTX);
		pOracle->svc = NULL;
	}

	if(pOracle->server != NULL)
	{
		OCIHandleFree((dvoid *)pOracle->server, OCI_HTYPE_SERVER);
		pOracle->server = NULL;
	}

	if(pOracle->err != NULL)
	{
		OCIHandleFree((dvoid *)pOracle->err, OCI_HTYPE_ERROR);
		pOracle->err = NULL;
	}

	if(pOracle->env != NULL)
	{
		OCIHandleFree((dvoid *)pOracle->env, OCI_HTYPE_ENV);
		pOracle->env = NULL;
	}
}

static dave_bool
_store_oracle_create_client(StoreOracle *pOracle, s8 *address, ub port, s8 *user, s8 *pwd, s8 *db)
{
	s8 conn_str[512];
	sword status;

	if((pOracle == NULL)
		|| (address == NULL)
		|| (user == NULL)
		|| (pwd == NULL)
		|| (db == NULL))
	{
		return dave_false;
	}

	dave_snprintf(conn_str, sizeof(conn_str), "//%s:%d/%s", address, port, db);

	status = OCIEnvCreate(&(pOracle->env), OCI_DEFAULT, NULL, NULL, NULL, NULL, 0, NULL);
	if(status != OCI_SUCCESS)
	{
		STABNOR("oracle env create failed! status:%d", status);
		_store_oracle_release_client(pOracle);
		return dave_false;
	}

	status = OCIHandleAlloc((dvoid *)pOracle->env, (dvoid **)&(pOracle->err), OCI_HTYPE_ERROR, 0, NULL);
	if(status != OCI_SUCCESS)
	{
		_store_oracle_release_client(pOracle);
		return dave_false;
	}

	status = OCIHandleAlloc((dvoid *)pOracle->env, (dvoid **)&(pOracle->server), OCI_HTYPE_SERVER, 0, NULL);
	if(status != OCI_SUCCESS)
	{
		_store_oracle_release_client(pOracle);
		return dave_false;
	}

	status = OCIHandleAlloc((dvoid *)pOracle->env, (dvoid **)&(pOracle->svc), OCI_HTYPE_SVCCTX, 0, NULL);
	if(status != OCI_SUCCESS)
	{
		_store_oracle_release_client(pOracle);
		return dave_false;
	}

	status = OCIServerAttach(pOracle->server, pOracle->err, (text *)conn_str, dave_strlen(conn_str), OCI_DEFAULT);
	if(status != OCI_SUCCESS)
	{
		s8 err_msg[1024];
		_store_oracle_error(pOracle, NULL, err_msg, sizeof(err_msg));
		STABNOR("oracle attach failed! connect:%s msg:%s", conn_str, err_msg);
		_store_oracle_release_client(pOracle);
		return dave_false;
	}

	status = OCIAttrSet((dvoid *)pOracle->svc, OCI_HTYPE_SVCCTX, (dvoid *)pOracle->server, 0, OCI_ATTR_SERVER, pOracle->err);
	if(status != OCI_SUCCESS)
	{
		_store_oracle_release_client(pOracle);
		return dave_false;
	}

	status = OCIHandleAlloc((dvoid *)pOracle->env, (dvoid **)&(pOracle->session), OCI_HTYPE_SESSION, 0, NULL);
	if(status != OCI_SUCCESS)
	{
		_store_oracle_release_client(pOracle);
		return dave_false;
	}

	status = OCIAttrSet((dvoid *)pOracle->session, OCI_HTYPE_SESSION, (dvoid *)user, dave_strlen(user), OCI_ATTR_USERNAME, pOracle->err);
	if(status != OCI_SUCCESS)
	{
		_store_oracle_release_client(pOracle);
		return dave_false;
	}

	status = OCIAttrSet((dvoid *)pOracle->session, OCI_HTYPE_SESSION, (dvoid *)pwd, dave_strlen(pwd), OCI_ATTR_PASSWORD, pOracle->err);
	if(status != OCI_SUCCESS)
	{
		_store_oracle_release_client(pOracle);
		return dave_false;
	}

	status = OCISessionBegin(pOracle->svc, pOracle->err, pOracle->session, OCI_CRED_RDBMS, OCI_DEFAULT);
	if(status != OCI_SUCCESS)
	{
		s8 err_msg[1024];
		_store_oracle_error(pOracle, NULL, err_msg, sizeof(err_msg));
		STABNOR("oracle session begin failed! user:%s msg:%s", user, err_msg);
		_store_oracle_release_client(pOracle);
		return dave_false;
	}

	status = OCIAttrSet((dvoid *)pOracle->svc, OCI_HTYPE_SVCCTX, (dvoid *)pOracle->session, 0, OCI_ATTR_SESSION, pOracle->err);
	if(status != OCI_SUCCESS)
	{
		_store_oracle_release_client(pOracle);
		return dave_false;
	}

	return dave_true;
}

static RetCode
_store_oracle_query(StoreOracle *pOracle, s8 *sql, void **ppJson, s8 *msg_ptr, ub msg_len)
{
	sword status;
	RetCode ret = RetCode_OK;
	OCIStmt *stmt = NULL;
	ub4 stmt_type = 0;
	ub4 column_number = 0;
	void *pJson = NULL;
	s8 **column_value = NULL;
	sb2 *column_ind = NULL;
	OCIDefine **column_define = NULL;
	ub4 column_index;
	dave_bool query_error = dave_false;
	sb4 error_code = 0;

	if(ppJson != NULL)
	{
		*ppJson = NULL;
	}

	if(msg_ptr != NULL)
	{
		msg_ptr[0] = '\0';
	}

	if(_store_oracle_is_connection_error(pOracle) == dave_true)
	{
		if(msg_ptr != NULL)
		{
			dave_strcpy(msg_ptr, "oracle disconnected", msg_len);
		}

		return RetCode_connect_error;
	}

	status = OCIHandleAlloc((dvoid *)pOracle->env, (dvoid **)&stmt, OCI_HTYPE_STMT, 0, NULL);
	if(status != OCI_SUCCESS)
	{
		if(msg_ptr != NULL)
		{
			dave_strcpy(msg_ptr, "oracle stmt alloc failed", msg_len);
		}
		return RetCode_db_sql_failed;
	}

	status = OCIStmtPrepare(stmt, pOracle->err, (text *)sql, dave_strlen(sql), OCI_NTV_SYNTAX, OCI_DEFAULT);
	if(status != OCI_SUCCESS)
	{
		_store_oracle_error(pOracle, &error_code, msg_ptr, msg_len);
		ret = RetCode_db_sql_failed;
		query_error = dave_true;
		goto _query_exit;
	}

	status = OCIAttrGet((dvoid *)stmt, OCI_HTYPE_STMT, &stmt_type, 0, OCI_ATTR_STMT_TYPE, pOracle->err);
	if(status != OCI_SUCCESS)
	{
		_store_oracle_error(pOracle, &error_code, msg_ptr, msg_len);
		ret = RetCode_db_sql_failed;
		query_error = dave_true;
		goto _query_exit;
	}

	if(stmt_type == OCI_STMT_SELECT)
	{
		status = OCIStmtExecute(pOracle->svc, stmt, pOracle->err, 0, 0, NULL, NULL, OCI_DEFAULT);
	}
	else
	{
		status = OCIStmtExecute(pOracle->svc, stmt, pOracle->err, 1, 0, NULL, NULL, OCI_DEFAULT);
	}

	if((status != OCI_SUCCESS) && (status != OCI_SUCCESS_WITH_INFO))
	{
		_store_oracle_error(pOracle, &error_code, msg_ptr, msg_len);

		if(error_code == 955)
		{
			ret = RetCode_table_exist;
		}
		else if((status == OCI_INVALID_HANDLE) || (status == OCI_STILL_EXECUTING))
		{
			ret = RetCode_connect_error;
		}
		else
		{
			ret = RetCode_db_sql_failed;
		}

		query_error = dave_true;
		goto _query_exit;
	}

	if(stmt_type == OCI_STMT_SELECT)
	{
		status = OCIAttrGet((dvoid *)stmt, OCI_HTYPE_STMT, &column_number, 0, OCI_ATTR_PARAM_COUNT, pOracle->err);
		if(status != OCI_SUCCESS)
		{
			_store_oracle_error(pOracle, &error_code, msg_ptr, msg_len);
			ret = RetCode_db_sql_failed;
			query_error = dave_true;
			goto _query_exit;
		}

		if(column_number > 0)
		{
			column_value = dave_ralloc(column_number * sizeof(s8 *));
			column_ind = dave_ralloc(column_number * sizeof(sb2));
			column_define = dave_ralloc(column_number * sizeof(OCIDefine *));

			dave_memset(column_value, 0x00, column_number * sizeof(s8 *));
			dave_memset(column_ind, 0x00, column_number * sizeof(sb2));
			dave_memset(column_define, 0x00, column_number * sizeof(OCIDefine *));

			for(column_index=0; column_index<column_number; column_index++)
			{
				column_value[column_index] = dave_ralloc(ORACLE_FETCH_VALUE_MAX);
				dave_memset(column_value[column_index], 0x00, ORACLE_FETCH_VALUE_MAX);

				status = OCIDefineByPos(stmt,
					&(column_define[column_index]),
					pOracle->err,
					column_index + 1,
					column_value[column_index],
					ORACLE_FETCH_VALUE_MAX,
					SQLT_STR,
					&(column_ind[column_index]),
					NULL,
					NULL,
					OCI_DEFAULT);
				if(status != OCI_SUCCESS)
				{
					_store_oracle_error(pOracle, &error_code, msg_ptr, msg_len);
					ret = RetCode_db_sql_failed;
					query_error = dave_true;
					goto _query_exit;
				}
			}

			pJson = dave_json_array_malloc();

			while((status = OCIStmtFetch2(stmt, pOracle->err, 1, OCI_FETCH_NEXT, 0, OCI_DEFAULT)) != OCI_NO_DATA)
			{
				void *pRow;

				if((status != OCI_SUCCESS) && (status != OCI_SUCCESS_WITH_INFO))
				{
					_store_oracle_error(pOracle, &error_code, msg_ptr, msg_len);
					ret = RetCode_db_sql_failed;
					query_error = dave_true;
					goto _query_exit;
				}

				pRow = dave_json_array_malloc();
				for(column_index=0; column_index<column_number; column_index++)
				{
					if(column_ind[column_index] == -1)
					{
						dave_json_array_add_str_len(pRow, "", 0);
					}
					else
					{
						dave_json_array_add_str_len(pRow, column_value[column_index], dave_strlen(column_value[column_index]));
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

_query_exit:
	if(stmt != NULL)
	{
		OCIHandleFree((dvoid *)stmt, OCI_HTYPE_STMT);
		stmt = NULL;
	}

	if(column_value != NULL)
	{
		for(column_index=0; column_index<column_number; column_index++)
		{
			if(column_value[column_index] != NULL)
			{
				dave_free(column_value[column_index]);
				column_value[column_index] = NULL;
			}
		}
		dave_free(column_value);
		column_value = NULL;
	}

	if(column_ind != NULL)
	{
		dave_free(column_ind);
		column_ind = NULL;
	}

	if(column_define != NULL)
	{
		dave_free(column_define);
		column_define = NULL;
	}

	if((query_error == dave_true) && (pJson != NULL))
	{
		dave_json_free(pJson);
		pJson = NULL;
	}

	return ret;
}

static void
_store_oracle_connect(StoreOracle *pOracle, s8 *address, ub port, s8 *user, s8 *pwd, s8 *db)
{
	dave_strcpy(pOracle->oracle_address, address, sizeof(pOracle->oracle_address));
	pOracle->oracle_port = port;
	dave_strcpy(pOracle->oracle_user, user, sizeof(pOracle->oracle_user));
	dave_strcpy(pOracle->oracle_pwd, pwd, sizeof(pOracle->oracle_pwd));
	dave_strcpy(pOracle->oracle_db, db, sizeof(pOracle->oracle_db));

	_store_oracle_release_client(pOracle);

	if(_store_oracle_create_client(pOracle,
		pOracle->oracle_address,
		pOracle->oracle_port,
		pOracle->oracle_user,
		pOracle->oracle_pwd,
		pOracle->oracle_db) == dave_false)
	{
		STABNOR("%s:%d %s/%s %s create client failed!",
			pOracle->oracle_address,
			pOracle->oracle_port,
			pOracle->oracle_user,
			pOracle->oracle_pwd,
			pOracle->oracle_db);
	}
	else
	{
		pOracle->connect_date = t_time_get_date(NULL);
	}

	oracle_statistics_reset(&(pOracle->statistics));
}

static dave_bool
_store_oracle_booting(void)
{
	s8 address[128];
	ub port;
	s8 user[128];
	s8 pwd[128];
	s8 db[128];
	ub oracle_index, boot_number;
	StoreOracle *pOracle;

	cfg_get_str(CFG_ORACLE_ADDRESS, address, sizeof(address), "127.0.0.1");
	port = cfg_get_ub(CFG_ORACLE_PORT, 1521);
	cfg_get_str(CFG_ORACLE_USER, user, sizeof(user), "system");
	cfg_get_str(CFG_ORACLE_PWD, pwd, sizeof(pwd), "123456");
	cfg_get_str(CFG_ORACLE_DBNAME, db, sizeof(db), "ORCLCDB");

	if((dave_strlen(address) == 0)
		|| (port == 0)
		|| (dave_strlen(user) == 0)
		|| (dave_strlen(pwd) == 0)
		|| (dave_strlen(db) == 0))
	{
		STLOG("invalid oracle config address:%s port:%d user:%s pwd:%s db:%s", address, port, user, pwd, db);
		return dave_true;
	}

	STLOG("address:%s port:%d user:%s pwd:%s db:%s", address, port, user, pwd, db);

	boot_number = 0;

	for(oracle_index=0; oracle_index<_oracle_number; oracle_index++)
	{
		pOracle = &_pOracle[oracle_index];

		if(_store_oracle_is_connection_error(pOracle) == dave_true)
		{
			_store_oracle_connect(pOracle, address, port, user, pwd, db);
		}

		if(_store_oracle_is_connection_error(pOracle) == dave_false)
		{
			boot_number ++;
		}
	}

	STLOG("%d/%d init success!", boot_number, _oracle_number);

	if(boot_number >= _oracle_number)
		return dave_true;
	else
		return dave_false;
}

static void
_store_oracle_boot_timer_out(TIMERID timer_id, ub thread_index)
{
	if((_store_oracle_booting() == dave_true)
		|| (++ _oracle_boot_times > 3))
	{
		base_timer_die(timer_id);
	}
}

static void
_store_oracle_init(ub oracle_number)
{
	_oracle_number = oracle_number;
	_oracle_boot_times = 0;

	_pOracle = dave_ralloc(_oracle_number * sizeof(StoreOracle));
}

static void
_store_oracle_exit(void)
{
	ub oracle_index;

	for(oracle_index=0; oracle_index<_oracle_number; oracle_index++)
	{
		_store_oracle_release_client(&_pOracle[oracle_index]);
		dave_memset(&_pOracle[oracle_index], 0x00, sizeof(StoreOracle));
	}

	dave_free(_pOracle);

	_pOracle = NULL;
}

static RetCode
_store_oracle_sql(MBUF **data, s8 *msg_ptr, ub msg_len, StoreOracle *pOracle, s8 *sql_ptr, ub sql_len)
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
		ret = _store_oracle_query(pOracle, sql_ptr, &pJson, msg_ptr, msg_len);
		if(ret == RetCode_connect_error)
		{
			STLOG("safe_counter:%d disconnect address:%s port:%d user:%s db:%s",
				safe_counter,
				pOracle->oracle_address, pOracle->oracle_port, pOracle->oracle_user, pOracle->oracle_db);

			_store_oracle_connect(pOracle,
				pOracle->oracle_address,
				pOracle->oracle_port,
				pOracle->oracle_user,
				pOracle->oracle_pwd,
				pOracle->oracle_db);

			if(_store_oracle_is_connection_error(pOracle) == dave_false)
			{
				STLOG("safe_counter:%d connect to address:%s port:%d user:%s db:%s",
					safe_counter,
					pOracle->oracle_address, pOracle->oracle_port, pOracle->oracle_user, pOracle->oracle_db);
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
store_oracle_init(ub thread_number)
{
	_store_oracle_init(thread_number);

	if(_store_oracle_booting() == dave_false)
	{
		base_timer_creat("soracleboot", _store_oracle_boot_timer_out, 6*1000);
	}
}

void
store_oracle_exit(void)
{
	_store_oracle_exit();
}

void
store_oracle_sql(ThreadId src, ub thread_index, StoreOracleReq *pReq)
{
	if(thread_index >= _oracle_number)
	{
		STABNOR("invalid thread_index:%d _oracle_number:%d", thread_index, _oracle_number);
		return;
	}

	StoreOracleRsp *pRsp = thread_reset_msg(pRsp);
	StoreOracle *pOracle = &_pOracle[thread_index];

	ub start_time = dave_os_time_us();
	pRsp->ret = _store_oracle_sql(&(pRsp->data), pRsp->msg, sizeof(pRsp->msg), pOracle, ms8(pReq->sql), mlen(pReq->sql));
	oracle_statistics(&(pOracle->statistics), dave_os_time_us() - start_time, src, ms8(pReq->sql));

	pRsp->ptr = pReq->ptr;

	if((pRsp->ret != RetCode_OK)
		&& (pRsp->ret != RetCode_table_exist)
		&& (pRsp->ret != RetCode_empty_data))
	{
		STLOG("%s execute index:%d/%d sql:%s ret:%s msg:%s",
			thread_name(src),
			thread_index, _oracle_number,
			ms8(pReq->sql),
			retstr(pRsp->ret),
			pRsp->msg);
	}

	id_msg(src, STORE_ORACLE_RSP, pRsp);

	dave_mfree(pReq->sql);
}

ub
store_oracle_info(s8 *info_ptr, ub info_len)
{
	ub info_index, oracle_index;
	StoreOracle *pOracle;
	s8 statistics_str[256];

	info_index = 0;

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "ORACLE INFO(%d):\n", _oracle_number);

	for(oracle_index=0; oracle_index<_oracle_number; oracle_index++)
	{
		pOracle = &_pOracle[oracle_index];

		oracle_statistics_info(statistics_str, sizeof(statistics_str), &(pOracle->statistics));

		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
			" %s:%d %s (%s) (%s)\n",
			pOracle->oracle_address, pOracle->oracle_port,
			_store_oracle_is_connection_error(pOracle) == dave_true ? "disconnect" : "connect",
			t_a2b_date_str(&(pOracle->connect_date)),
			statistics_str);
	}

	return info_index;
}

#else

void store_oracle_init(ub thread_number)
{

}

void store_oracle_exit(void)
{

}

void
store_oracle_sql(ThreadId src, ub thread_index, StoreOracleReq *pReq)
{
	STLOG("store_oracle_sql is disabled");
}

ub
store_oracle_info(s8 *info_ptr, ub info_len)
{
	ub info_index = 0;

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "ORACLE INFO(DISABLED)\n");

	return info_index;
}

#endif

#endif

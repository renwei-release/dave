/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_STORE__
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "store_msg.h"
#include "store_mysql.h"
#include "store_log.h"

static void
_store_mysql_debug(s8 *rsp_ptr, ub rsp_len)
{
	s8 *address = "192.168.220.130";
	ub port = 3306;
	s8 *user = "root";
	s8 *pwd = "CWLtc14@#!";
	s8 *db_name = "mysql";
	void *pSql;
	SqlRet ret;

	pSql = dave_mysql_creat_client(address, port, user, pwd, db_name);

	dave_snprintf(rsp_ptr, rsp_len, "connect %s:%d account:%s/%s db:%s %s!",
		address, port, user, pwd, db_name,
		pSql==NULL?"failed":"success");

	if(pSql != NULL)
	{
		ret = dave_mysql_query(pSql, "CREATE DATABASE DB0001");
		STLOG("ret:%s data:%s", retstr(ret.ret), dave_json_to_string(ret.pJson, NULL));
		dave_mysql_free_ret(ret);

		ret = dave_mysql_query(pSql, "SELECT help_topic_id, name, help_category_id, description, example, url FROM help_topic WHERE name = 'AREA';");
		STLOG("ret:%s data:%s", retstr(ret.ret), dave_json_to_string(ret.pJson, NULL));
		dave_mysql_free_ret(ret);

		dave_mysql_release_client(pSql);
	}
}

// =====================================================================

void
store_debug(ThreadId src, DebugReq *pReq)
{
	DebugRsp *pRsp = thread_reset_msg(pRsp);

	if(dave_strcmp(pReq->msg, "mysql") == dave_true)
	{
		_store_mysql_debug(pRsp->msg, sizeof(pRsp->msg));
	}

	pRsp->ptr = pReq->ptr;

	id_msg(src, MSGID_DEBUG_RSP, pRsp);
}

#endif


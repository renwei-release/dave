/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(MYSQL_3RDPARTY)
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <dlfcn.h>
#include "mysql.h"
#include "mysqld_error.h"
#include "errmsg.h"
#include "dave_tools.h"
#include "dave_mysql.h"
#include "dave_json.h"
#include "party_log.h"

#define MYSQL_RECONNECT_ENABLE 1
#define MYSQL_AUTO_COMMIT_ENABLE 1

#define ER_ALREADY_CONNECTED 2058 // This handle is already connected

static const char dll_file_table[][64] = {
	{"/usr/lib64/libmysqlclient.so"},
	{"/usr/lib64/mysql/libmysqlclient.so"},
	{"/usr/lib/x86_64-linux-gnu/libmysqlclient.so"},
	{""}
};

static void *_mysql_dll_handle = NULL;

static MYSQL * STDCALL (* so_mysql_init)(MYSQL *mysql) = NULL;
static void STDCALL (* so_mysql_close)(MYSQL *sock) = NULL;
static void	STDCALL (* so_mysql_free_result)(MYSQL_RES *result) = NULL;
static MYSQL * STDCALL (* so_mysql_real_connect)(MYSQL *mysql, const char *host, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long clientflag) = NULL;
static unsigned int STDCALL (* so_mysql_errno)(MYSQL *mysql) = NULL;
static const char * STDCALL (* so_mysql_error)(MYSQL *mysql) = NULL;
static int STDCALL (* so_mysql_select_db)(MYSQL *mysql, const char *db) = NULL;
static int STDCALL (* so_mysql_next_result)(MYSQL *mysql) = NULL;
static MYSQL_RES * STDCALL (* so_mysql_store_result)(MYSQL *mysql) = NULL;
static MYSQL_RES * STDCALL (* so_mysql_use_result)(MYSQL *mysql) = NULL;
static int STDCALL (* so_mysql_query)(MYSQL *mysql, const char *q) = NULL;
static my_ulonglong STDCALL (* so_mysql_num_rows)(MYSQL_RES *res) = NULL;
static unsigned int STDCALL (* so_mysql_num_fields)(MYSQL_RES *res) = NULL;
static unsigned long * STDCALL (* so_mysql_fetch_lengths)(MYSQL_RES *result) = NULL;
static MYSQL_ROW STDCALL (* so_mysql_fetch_row)(MYSQL_RES *result) = NULL;
static int STDCALL (* so_mysql_options)(MYSQL *mysql, enum mysql_option option, const void *arg) = NULL;
static int STDCALL (* so_mysql_ping)(MYSQL *mysql);
static my_bool STDCALL (* so_mysql_autocommit)(MYSQL * mysql, my_bool auto_mode);
static my_bool STDCALL (* so_mysql_commit)(MYSQL * mysql);
static int STDCALL (* so_mysql_set_character_set)(MYSQL *mysql, const char *csname);
static const char * STDCALL (* so_mysql_character_set_name)(MYSQL *mysql);
static MYSQL_FIELD * STDCALL (* so_mysql_fetch_field)(MYSQL_RES *result);
static unsigned long STDCALL (* so_mysql_real_escape_string)(MYSQL *mysql,char *to,const char *from,unsigned long length);

static dave_bool
_mysql_dll_init(void)
{
	sb dll_file_table_index;

	if(_mysql_dll_handle == NULL)
	{
		for(dll_file_table_index=0; (_mysql_dll_handle==NULL)&&(dll_file_table_index<16); dll_file_table_index++)
		{
			if(dave_strlen(dll_file_table[dll_file_table_index]) == 0)
				break;

			_mysql_dll_handle = dlopen(dll_file_table[dll_file_table_index], RTLD_LAZY);
		}
	}

	if(_mysql_dll_handle == NULL)
	{
		PARTYABNOR("can not find any libmysqlclient.so!!");
		return dave_false;
	}

	so_mysql_init = dlsym(_mysql_dll_handle, "mysql_init");
	if(so_mysql_init == NULL)
		return dave_false;
	so_mysql_close = dlsym(_mysql_dll_handle, "mysql_close");
	if(so_mysql_close == NULL)
		return dave_false;
	so_mysql_free_result = dlsym(_mysql_dll_handle, "mysql_free_result");
	if(so_mysql_free_result == NULL)
		return dave_false;
	so_mysql_real_connect = dlsym(_mysql_dll_handle, "mysql_real_connect");
	if(so_mysql_real_connect == NULL)
		return dave_false;
	so_mysql_errno = dlsym(_mysql_dll_handle, "mysql_errno");
	if(so_mysql_errno == NULL)
		return dave_false;
	so_mysql_error = dlsym(_mysql_dll_handle, "mysql_error");
	if(so_mysql_error == NULL)
		return dave_false;
	so_mysql_select_db = dlsym(_mysql_dll_handle, "mysql_select_db");
	if(so_mysql_select_db == NULL)
		return dave_false;
	so_mysql_next_result = dlsym(_mysql_dll_handle, "mysql_next_result");
	if(so_mysql_next_result == NULL)
		return dave_false;
	so_mysql_store_result = dlsym(_mysql_dll_handle, "mysql_store_result");
	if(so_mysql_store_result == NULL)
		return dave_false;
	so_mysql_use_result = dlsym(_mysql_dll_handle, "mysql_use_result");
	if(so_mysql_use_result == NULL)
		return dave_false;
	so_mysql_query = dlsym(_mysql_dll_handle, "mysql_query");
	if(so_mysql_query == NULL)
		return dave_false;
	so_mysql_num_rows = dlsym(_mysql_dll_handle, "mysql_num_rows");
	if(so_mysql_num_rows == NULL)
		return dave_false;
	so_mysql_num_fields = dlsym(_mysql_dll_handle, "mysql_num_fields");
	if(so_mysql_num_fields == NULL)
		return dave_false;
	so_mysql_fetch_lengths = dlsym(_mysql_dll_handle, "mysql_fetch_lengths");
	if(so_mysql_fetch_lengths == NULL)
		return dave_false;
	so_mysql_fetch_row = dlsym(_mysql_dll_handle, "mysql_fetch_row");
	if(so_mysql_fetch_row == NULL)
		return dave_false;
	so_mysql_options = dlsym(_mysql_dll_handle, "mysql_options");
	if(so_mysql_options == NULL)
		return dave_false;
	so_mysql_ping = dlsym(_mysql_dll_handle, "mysql_ping");
	if(so_mysql_ping == NULL)
		return dave_false;
	so_mysql_autocommit = dlsym(_mysql_dll_handle, "mysql_autocommit");
	if(so_mysql_autocommit == NULL)
		return dave_false;
	so_mysql_commit = dlsym(_mysql_dll_handle, "mysql_commit");
	if(so_mysql_commit == NULL)
		return dave_false;
	so_mysql_set_character_set = dlsym(_mysql_dll_handle, "mysql_set_character_set");
	if(so_mysql_set_character_set == NULL)
		return dave_false;
	so_mysql_character_set_name = dlsym(_mysql_dll_handle, "mysql_character_set_name");
	if(so_mysql_character_set_name == NULL)
		return dave_false;
	so_mysql_fetch_field = dlsym(_mysql_dll_handle, "mysql_fetch_field");
	if(so_mysql_fetch_field == NULL)
		return dave_false;
	so_mysql_real_escape_string = dlsym(_mysql_dll_handle, "mysql_real_escape_string");
	if(so_mysql_real_escape_string == NULL)
			return dave_false;

	return dave_true;
}

static void
_mysql_dll_exit(void)
{
	if(_mysql_dll_handle != NULL)
	{
		dlclose(_mysql_dll_handle);
		_mysql_dll_handle = NULL;

		so_mysql_init = NULL;
		so_mysql_close = NULL;
		so_mysql_free_result = NULL;
		so_mysql_real_connect = NULL;
		so_mysql_errno = NULL;
		so_mysql_error = NULL;
		so_mysql_select_db = NULL;
		so_mysql_next_result = NULL;
		so_mysql_store_result = NULL;
		so_mysql_query = NULL;
		so_mysql_num_rows = NULL;
		so_mysql_num_fields = NULL;
		so_mysql_fetch_lengths = NULL;
		so_mysql_fetch_row = NULL;
		so_mysql_options = NULL;
		so_mysql_ping = NULL;
		so_mysql_set_character_set = NULL;
		so_mysql_real_escape_string = NULL;
	}
}

static void *
_mysql_client_init(MYSQL *pSql)
{
	MYSQL *mysql;
	char value;

	mysql = so_mysql_init(pSql);

	if(pSql != mysql)
	{
		PARTYABNOR("ptr has error! %lx %lx", pSql, mysql);
	}

	if(mysql != NULL)
	{
		value = 6;		// seconds
		so_mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &value);
		value = 1;		// enable
		so_mysql_options(mysql, MYSQL_OPT_RECONNECT, &value);
		value =  15;	// seconds
		so_mysql_options(mysql, MYSQL_OPT_READ_TIMEOUT, &value);
		value =  15;	// seconds
		so_mysql_options(mysql, MYSQL_OPT_WRITE_TIMEOUT, &value);
	}

	return pSql;
}

static void
_mysql_client_exit(MYSQL *pSql)
{
	so_mysql_close(pSql);
}

static RetCode
_mysql_connect(MYSQL *pSql, s8 *address, ub port, s8 *user, s8 *pwd, s8 *db_name)
{
	unsigned int errcode;
	char value;

	if(so_mysql_real_connect(pSql, (const char *)address,
		(const char *)user, (const char *)pwd, (const char *)db_name,
		(unsigned int)port, NULL, CLIENT_MULTI_STATEMENTS|CLIENT_MULTI_RESULTS) == NULL)
	{
		errcode = so_mysql_errno(pSql);

		if(ER_BAD_DB_ERROR == errcode)
		{
			return RetCode_db_not_find;
		}
		else if(ER_ALREADY_CONNECTED == errcode)
		{
			PARTYLOG("mysql connect error:(%d)%s, address:%s port:%d user:%s pwd:%s db_name:%s",
				errcode, so_mysql_error(pSql), address, port, user, pwd, db_name);
		}
		else
		{
			PARTYABNOR("mysql connect error:(%d)%s, address:%s port:%d user:%s pwd:%s db_name:%s",
				errcode, so_mysql_error(pSql), address, port, user, pwd, db_name);

			return RetCode_connect_error;
		}
	}

	value = MYSQL_RECONNECT_ENABLE;
	so_mysql_options(pSql, MYSQL_OPT_RECONNECT, &value);

	// open auto commit!
	so_mysql_autocommit(pSql, MYSQL_AUTO_COMMIT_ENABLE);

	if(so_mysql_set_character_set(pSql, "utf8") != 0)
	{
		PARTYABNOR("mysql set character:%s<%d>", so_mysql_error(pSql), so_mysql_errno(pSql));
	}

	return RetCode_OK;
}

static RetCode
_mysql_select_db(MYSQL *pSql, s8 *db_name)
{
	if(so_mysql_select_db(pSql, (const char *)db_name) != 0)
	{
		PARTYABNOR("mysql select db error:%s<%d>", so_mysql_error(pSql), so_mysql_errno(pSql));
		return RetCode_db_sql_failed;		
	}
	else
	{
		return RetCode_OK;
	}
}

static RetCode
_mysql_query(MYSQL *pSql, s8 *sql)
{
	ub safe_counter;
	unsigned int errcode;

	safe_counter = 0;
	do {
		so_mysql_free_result(so_mysql_store_result(pSql));
	} while(((safe_counter ++) < 102400) && (so_mysql_next_result(pSql) == 0));
	if(safe_counter >= 102400)
	{
		PARTYABNOR("safe_counter:%d sql:%s find error! mysql_next_result=%d",
			safe_counter, sql, so_mysql_next_result(pSql));
	}

	if(so_mysql_query(pSql, (const char *)sql) != 0)
	{
		errcode = so_mysql_errno(pSql);

		if(ER_TABLE_EXISTS_ERROR == errcode)
			return RetCode_table_exist;
		else if((CR_SERVER_LOST == errcode) || (CR_SERVER_GONE_ERROR == errcode))
			return RetCode_connect_error;
		else if(ER_DB_CREATE_EXISTS == errcode)
			return RetCode_OK;
		else
			return RetCode_db_sql_failed;
	}

	return RetCode_OK;
}

static void *
_mysql_query_to_json(ub row_num, ub fields_num, MYSQL_RES *res)
{
	MYSQL_ROW row;
	unsigned long *lengths;
	ub fields_index;
	void *pJson = dave_json_array_malloc();
	void *pArray;

	while((row = so_mysql_fetch_row(res)))
	{
		lengths = so_mysql_fetch_lengths(res);

		pArray = dave_json_array_malloc();

		for(fields_index=0; fields_index<fields_num; fields_index++)
		{
			dave_json_array_add_str_len(pArray, row[fields_index], (ub)(lengths[fields_index]));
		}

		dave_json_array_add_object(pJson, pArray);
	}

	return pJson;
}

static SqlRet
_mysql_query_ret(MYSQL *pSql, s8 *sql)
{
	MYSQL_RES *res;
	ub row_num, fields_num;
	SqlRet ret  = { RetCode_OK, NULL };

	res = so_mysql_store_result(pSql);
	if(res == NULL)
	{
		PARTYDEBUG("sql:%s get res failed!", sql);
		return ret;
	}

	row_num = so_mysql_num_rows(res);
	fields_num = so_mysql_num_fields(res);
	if((row_num == 0) || (fields_num == 0))
	{
		ret.pJson = NULL;
	}
	else
	{
		ret.pJson = _mysql_query_to_json(row_num, fields_num, res);
	}

	return ret;
}

// =====================================================================

void
dave_mysql_init(void)
{
	if(_mysql_dll_init() == dave_false)
	{
		DAVEASSERT(0, "mysql dll failed! %s", dll_file_table[0]);
	}
}

void
dave_mysql_exit(void)
{
	_mysql_dll_exit();
}

void *
dave_mysql_creat_client(s8 *address, ub port, s8 *user, s8 *pwd, s8 *db_name)
{
	MYSQL *pSql = dave_malloc(sizeof(MYSQL));
	RetCode ret;

	_mysql_client_init(pSql);

	ret = _mysql_connect(pSql, address, port, user, pwd, db_name);
	if(ret != RetCode_OK)
	{
		if(ret != RetCode_db_not_find)
			PARTYABNOR("mysql connect error:%s<%d>",
				so_mysql_error(pSql), so_mysql_errno(pSql));
		dave_mysql_release_client(pSql); pSql = NULL;
		return NULL;
	}

	return (void *)pSql;
}

void
dave_mysql_release_client(void *pSql)
{
	if(pSql != NULL)
	{
		_mysql_client_exit(pSql);

		dave_free(pSql);
	}
}

RetCode
dave_mysql_select_db(void *pSql, s8 *db_name)
{
	return _mysql_select_db(pSql, db_name);
}

SqlRet
dave_mysql_query(void *pSql, s8 *sql)
{
	SqlRet ret  = { RetCode_Unknown_error, NULL };

	if(pSql == NULL)
	{
		PARTYLOG("sql:%s has empty pSql!", sql);
		ret.ret = RetCode_connect_error;
		return ret;
	}

	ret.ret = _mysql_query(pSql, sql);
	if(ret.ret == RetCode_table_exist)
	{
		ret.ret = RetCode_OK;
		return ret;
	}
	else if(ret.ret != RetCode_OK)
	{
		PARTYLOG("sql:%s query error<%d>:%s/%s",
			sql, so_mysql_errno(pSql), so_mysql_error(pSql),
			retstr(ret.ret));
	}
	else
	{
		ret = _mysql_query_ret(pSql, sql);
	}

	return ret;
}

void
dave_mysql_free_ret(SqlRet ret)
{
	if(ret.pJson != NULL)
	{
		dave_free(ret.pJson);
	}
}

s8 *
dave_mysql_error(void *pSql)
{
	return (s8 *)so_mysql_error(pSql);
}

#endif


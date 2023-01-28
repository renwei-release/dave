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
#include "dave_tools.h"
#include "party_log.h"

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

#endif


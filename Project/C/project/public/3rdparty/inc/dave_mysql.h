/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_MYSQL_H__
#define __DAVE_MYSQL_H__
#include "dave_define.h"

void dave_mysql_init(void);

void dave_mysql_exit(void);

void * dave_mysql_creat_client(s8 *address, ub port, s8 *user, s8 *pwd, s8 *db_name);

void dave_mysql_release_client(void *pSql);

RetCode dave_mysql_select_db(void *pSql, s8 *db_name);

typedef struct {
	RetCode ret;
	void *pJson;
} SqlRet;

SqlRet dave_mysql_query(void *pSql, s8 *sql);

void dave_mysql_free_ret(SqlRet ret);

s8 * dave_mysql_error(void *pSql);

void dave_mysql_ping(void *mysql);

ErrCode dave_mysql_raw_query(void *mysql, s8 *sql);
void * dave_mysql_malloc_result(void *mysql);
void dave_mysql_free_result(void *mysql, void *res);
ub dave_mysql_result_rows(void *res);
ub dave_mysql_result_fields(void *res);
s8 ** dave_mysql_fetch_row(void *res);
void * dave_mysql_fetch_field(void *res);
unsigned long * dave_mysql_fetch_length(void *res);

#endif


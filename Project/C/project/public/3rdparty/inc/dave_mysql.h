/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_MYSQL_H__
#define __DAVE_MYSQL_H__

void dave_mysql_init(void);

void dave_mysql_exit(void);

void * dave_mysql_creat_client(s8 *address, ub port, s8 *user, s8 *pwd, s8 *db_name);

void dave_mysql_release_client(void *pSql);

typedef struct {
	RetCode ret;
	void *pJson;
} SqlRet;

SqlRet dave_mysql_query(void *pSql, s8 *sql);

void dave_mysql_free_ret(SqlRet ret);

#endif


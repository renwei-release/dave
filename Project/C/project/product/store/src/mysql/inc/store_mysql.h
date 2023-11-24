/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __STORE_MYSQL_H__
#define __STORE_MYSQL_H__

void store_mysql_init(ub thread_number);

void store_mysql_exit(void);

void store_mysql_sql(ThreadId src, ub thread_index, StoreMysqlReq *pReq);

ub store_mysql_info(s8 *info_ptr, ub info_len);

#endif


/*
 * Copyright (c) 2026 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __STORE_PGSQL_H__
#define __STORE_PGSQL_H__

void store_pgsql_init(ub thread_number);

void store_pgsql_exit(void);

void store_pgsql_sql(ThreadId src, ub thread_index, StorePgsqlReq *pReq);

ub store_pgsql_info(s8 *info_ptr, ub info_len);

#endif


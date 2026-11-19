/*
 * Copyright (c) 2026 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __STORE_ORACLE_H__
#define __STORE_ORACLE_H__

void store_oracle_init(ub thread_number);

void store_oracle_exit(void);

void store_oracle_sql(ThreadId src, ub thread_index, StoreOracleReq *pReq);

ub store_oracle_info(s8 *info_ptr, ub info_len);

#endif

/*
 * ================================================================================
 * (c) Copyright 2021 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2021.08.23.
 * ================================================================================
 */

#ifndef __THREAD_REMOTE_ID_TABLE_H__
#define __THREAD_REMOTE_ID_TABLE_H__
#include "base_macro.h"

void thread_remote_id_table_init(void);

void thread_remote_id_table_exit(void);

void thread_remote_id_table_add(ThreadId remote_id, s8 *remote_name);

void thread_remote_id_table_del(ThreadId remote_id, s8 *remote_name);

dave_bool thread_remote_id_table_inq(ThreadId remote_id);

#endif


/*
 * ================================================================================
 * (c) Copyright 2022 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2022.02.23.
 * ================================================================================
 */

#ifndef __THREAD_GID_TABLE_H__
#define __THREAD_GID_TABLE_H__

void thread_gid_table_init(void);

void thread_gid_table_exit(void);

void thread_gid_table_add(s8 *gid, s8 *thread_name, ThreadId remote_id);

void thread_gid_table_del(s8 *gid, s8 *thread_name);

ThreadId thread_gid_table_inq(s8 *gid, s8 *thread_name);

#endif


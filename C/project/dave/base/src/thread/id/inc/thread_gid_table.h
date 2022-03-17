/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_GID_TABLE_H__
#define __THREAD_GID_TABLE_H__

void thread_gid_table_init(void);

void thread_gid_table_exit(void);

void thread_gid_table_add(s8 *gid, s8 *thread_name, ThreadId remote_id);

void thread_gid_table_del(s8 *gid, s8 *thread_name);

ThreadId thread_gid_table_inq(s8 *gid, s8 *thread_name);

#endif


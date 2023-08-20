/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_verno.h"
#include "dave_tools.h"
#include "base_tools.h"
#include "thread_sync.h"
#include "sync_lock.h"

#ifdef SYNC_STACK_CLIENT
extern void sync_client_init(s8 *sync_domain);
extern void sync_client_exit(void);
extern ThreadId sync_client_thread_id(ThreadId thread_id);
extern dave_bool sync_client_gid_ready(s8 *gid);
#endif
#ifdef SYNC_STACK_SERVER
extern void sync_server_init(void);
extern void sync_server_exit(void);
#endif

// =====================================================================

void
thread_sync_init(s8 *sync_domain)
{
#if defined(SYNC_STACK_CLIENT) || defined(SYNC_STACK_SERVER)
	sync_lock_init();
#endif

#ifdef SYNC_STACK_CLIENT
	if(T_sync_type() == SYNC_CLIENT)
	{
		sync_client_init(sync_domain);
	}
#endif
#ifdef SYNC_STACK_SERVER
	if(T_sync_type() == SYNC_SERVER)
	{
		sync_server_init();
	}
#endif
}

void
thread_sync_exit(void)
{
#ifdef SYNC_STACK_SERVER
	if(T_sync_type() == SYNC_SERVER)
	{
		sync_server_exit();
	}
#endif
#ifdef SYNC_STACK_CLIENT
	if(T_sync_type() == SYNC_CLIENT)
	{
		sync_client_exit();
	}
#endif

#if defined(SYNC_STACK_CLIENT) || defined(SYNC_STACK_SERVER)
    sync_lock_exit();
#endif
}

ThreadId
thread_sync_thread_id(ThreadId thread_id)
{
#ifdef SYNC_STACK_CLIENT
	return sync_client_thread_id(thread_id);
#else
	return thread_id;
#endif
}

dave_bool
thread_sync_gid_ready(s8 *gid)
{
#ifdef SYNC_STACK_CLIENT
	return sync_client_gid_ready(gid);
#else
	return dave_false;
#endif
}

#endif


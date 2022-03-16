/*
 * ================================================================================
 * (c) Copyright 2017 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2017.04.26.
 * The synchronization service in the future may try to connect to all within
 * the framework of the server.
 * ================================================================================
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_verno.h"
#include "dave_tools.h"
#include "base_tools.h"
#include "thread_sync.h"
#include "sync_lock.h"

#ifdef SYNC_STACK_CLIENT
extern void sync_client_init(void);
extern void sync_client_exit(void);
#endif
#ifdef SYNC_STACK_SERVER
extern void sync_server_init(void);
extern void sync_server_exit(void);
#endif

// =====================================================================

void
thread_sync_init(void)
{
#if defined(SYNC_STACK_CLIENT) || defined(SYNC_STACK_SERVER)
	sync_lock_init();
#endif
    
#ifdef SYNC_STACK_CLIENT
	if(T_sync_type() == SYNC_CLIENT)
	{
		sync_client_init();
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

#endif

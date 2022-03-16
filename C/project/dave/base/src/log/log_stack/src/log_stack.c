/*
 * ================================================================================
 * (c) Copyright 2017 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2017.04.01.
 * ================================================================================
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_verno.h"
#include "dave_tools.h"
#include "log_stack.h"

#ifdef LOG_STACK_CLIENT
extern void log_stack_client_init(void);
extern void log_stack_client_exit(void);
#endif
#ifdef LOG_STACK_SERVER
extern void log_stack_server_init(void);
extern void log_stack_server_exit(void);
#endif

typedef enum {
	LOG_SERVER,
	LOG_CLIENT,
	LOG_MAX,
} LOGType;

#if defined(LOG_STACK_CLIENT) || defined(LOG_STACK_SERVER)
static LOGType
_thread_log_server_flag(void)
{
	s8 *product = dave_verno_product(NULL, NULL, 0);
	
	if(dave_strcmp(product, "LOG") == dave_true)
	{
		return LOG_SERVER;
	}
	else
	{
		return LOG_CLIENT;
	}
}
#endif

// =====================================================================

void
log_stack_init(void)
{
#ifdef LOG_STACK_CLIENT
	if(_thread_log_server_flag() == LOG_CLIENT)
	{
		log_stack_client_init();
	}
#endif
#ifdef LOG_STACK_SERVER
	if(_thread_log_server_flag() == LOG_SERVER)
	{
		log_stack_server_init();
	}
#endif
}

void
log_stack_exit(void)
{
#ifdef LOG_STACK_SERVER
	if(_thread_log_server_flag() == LOG_SERVER)
	{
		log_stack_server_exit();
	}
#endif
#ifdef LOG_STACK_CLIENT
	if(_thread_log_server_flag() == LOG_CLIENT)
	{
		log_stack_client_exit();
	}
#endif
}

#endif


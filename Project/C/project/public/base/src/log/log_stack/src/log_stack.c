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
#include "log_stack.h"

#ifdef LOG_STACK_CLIENT
extern void log_client_init(void);
extern void log_client_exit(void);
#endif
#ifdef LOG_STACK_SERVER
extern void log_server_init(void);
extern void log_server_exit(void);
#endif

// =====================================================================

void
log_stack_init(void)
{
#ifdef LOG_STACK_CLIENT
	log_client_init();
#endif
#ifdef LOG_STACK_SERVER
	log_server_init();
#endif
}

void
log_stack_exit(void)
{
#ifdef LOG_STACK_SERVER
	log_server_exit();
#endif
#ifdef LOG_STACK_CLIENT
	log_client_exit();
#endif
}

#endif


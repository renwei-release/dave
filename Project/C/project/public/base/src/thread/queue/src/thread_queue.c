/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_verno.h"
#include "dave_tools.h"
#include "base_tools.h"
#include "thread_queue.h"

#ifdef QUEUE_STACK_CLIENT
extern void queue_client_init(void);
extern void queue_client_exit(void);
#endif
#ifdef QUEUE_STACK_SERVER
extern void queue_server_init(void);
extern void queue_server_exit(void);
#endif

// =====================================================================

void
thread_queue_init(void)
{
#ifdef QUEUE_STACK_CLIENT
	queue_client_init();
#endif
#ifdef QUEUE_STACK_SERVER
	queue_server_init();
#endif
}

void
thread_queue_exit(void)
{
#ifdef QUEUE_STACK_SERVER
	queue_server_exit();
#endif
#ifdef QUEUE_STACK_CLIENT
	queue_client_exit();
#endif
}

#endif


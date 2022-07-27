/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "log_stack.h"
#ifdef LOG_STACK_SERVER
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "thread_chain.h"
#include "log_lock.h"
#include "log_log.h"

// =====================================================================

void
tracing_logic(void *pArrayJson)
{
	LOGLOG("%s", dave_json_to_string(pArrayJson, NULL));
}

#endif


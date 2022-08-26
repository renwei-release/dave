/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "log_stack.h"
#ifdef LOG_STACK_SERVER
#include <dlfcn.h>
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "base_tools.h"

// =====================================================================

void
log_save_cfg_reset(void)
{
	chain_config_reset(NULL);
}

dave_bool
log_save_type_enable(ChainType type)
{
	return chain_config_type_enable(type);
}

#endif


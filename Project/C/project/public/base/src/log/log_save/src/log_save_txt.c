/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "log_stack.h"
#if defined(LOG_STACK_SERVER) || defined(LOG_STACK_CLIENT)
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "log_save_tools.h"
#include "log_lock.h"
#include "log_log.h"

// =====================================================================

void
log_save_txt(sb file_id, TraceLevel level, s8 *content_ptr, ub content_len)
{
	sb file_len;

	file_len = dave_os_file_len(READ_FLAG, NULL, file_id);
	if(file_len < 0)
	{
		file_len = 0;
	}

	dave_os_file_save(file_id, (ub)file_len, content_len, (u8 *)content_ptr);
}

#endif


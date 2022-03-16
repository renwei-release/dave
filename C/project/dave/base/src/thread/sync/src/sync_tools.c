/*
 * ================================================================================
 * (c) Copyright 2021 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2021.11.06.
 * The synchronization service in the future may try to connect to all within
 * the framework of the server.
 *
 * ================================================================================
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_CLIENT) || defined(SYNC_STACK_SERVER)
#include "dave_base.h"
#include "dave_tools.h"

// =====================================================================

s8 *
sync_work_start_second_str(ub work_start_second, s8 *str_ptr, ub str_len)
{
	if(work_start_second == 0)
		dave_snprintf(str_ptr, str_len, "XXX");
	else if(work_start_second < 60)
		dave_snprintf(str_ptr, str_len, "%02ds", work_start_second);
	else if(work_start_second < 3600)
		dave_snprintf(str_ptr, str_len, "%02dm", work_start_second/60);
	else
		dave_snprintf(str_ptr, str_len, "%03dh", work_start_second/3600);

	return str_ptr;
}

#endif


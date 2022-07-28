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
#include "log_save_json.h"
#include "tracing_level.h"
#include "log_log.h"

static void
_tracing_update_ub(void *pJson, s8 *key)
{
	ub ub_data;
	s8 ub_str[128];
	ub ub_len;

	if(dave_json_get_ub(pJson, key, &ub_data) == dave_true)
	{
		ub_len = dave_snprintf(ub_str, sizeof(ub_str), "%lu", ub_data);

		dave_json_add_str_len(pJson, key, ub_str, ub_len);
	}
}

static void
_tracing_fixed_bug(void *pArrayJson)
{
	sb array_length = dave_json_get_array_length(pArrayJson);
	sb array_index;
	void *pJson;

	for(array_index=0; array_index<array_length; array_index++)
	{
		pJson = dave_json_get_array_idx(pArrayJson, array_index);
		if(pJson != NULL)
		{
			_tracing_update_ub(pJson, (s8 *)JSON_LOG_call_id);
		}
	}
}

// =====================================================================

/*
 * 似乎Jaerer有一个BUG，不能显示很大的整数，这里转换成字符串显示
 */
void
tracing_fixed_bug(void *pArrayJson)
{
	_tracing_fixed_bug(pArrayJson);
}

#endif


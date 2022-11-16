/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "thread_struct.h"
#include "thread_quit.h"
#include "thread_id.h"
#include "thread_guardian.h"
#include "thread_statistics_param.h"
#include "thread_log.h"

#define DISPLAY_MSG_ID_MAX 128

static ub
_thread_statistics_info_the_msg_id(void *statistics_kv, ub msg_id, s8 *msg_ptr, ub msg_len)
{
	ub msg_index, index;
	ThreadStatistics *pStatistics;
	dave_bool frist_output = dave_true;

	msg_index = 0;

	msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index, " %s\n", msgstr(msg_id));

	for(index=0; index<102400; index++)
	{
		pStatistics = kv_index_key_ptr(statistics_kv, index);
		if(pStatistics == NULL)
			break;	

		if(pStatistics->msg_id == msg_id)
		{
			if(frist_output == dave_false)
			{
				msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index, "\n");
			}			
			frist_output = dave_false;

			msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index,
				"      %s->%s:%d",
				thread_name(pStatistics->msg_src),
				thread_name(pStatistics->msg_dst),
				pStatistics->msg_counter);
		}
	}

	return msg_index;
}

static dave_bool
_thread_statistics_info_is_display_msg_id(ub *display_msg_id_table, ub msg_id)
{
	ub table_index, empty_table_index;

	empty_table_index = DISPLAY_MSG_ID_MAX;

	for(table_index=0; table_index<DISPLAY_MSG_ID_MAX; table_index++)
	{
		if(display_msg_id_table[table_index] == msg_id)
		{
			return dave_true;
		}

		if(display_msg_id_table[table_index] == 0)
		{
			empty_table_index = table_index;
		}
	}

	if(empty_table_index < DISPLAY_MSG_ID_MAX)
	{
		display_msg_id_table[empty_table_index] = msg_id;
	}
	else
	{
		THREADABNOR("display msg id table overflow:%d/%d!", DISPLAY_MSG_ID_MAX, empty_table_index);
	}

	return dave_false;
}

static ub
_thread_statistics_info_api(void *statistics_kv, s8 *msg_ptr, ub msg_len)
{
	ub display_msg_id_table[DISPLAY_MSG_ID_MAX];
	ub msg_index, index;
	ThreadStatistics *pStatistics;
	dave_bool frist_output = dave_true;

	dave_memset(display_msg_id_table, 0x00, sizeof(display_msg_id_table));

	msg_index = 0;

	msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index, "THREAD STATISTICS INFO:\n");
	
	for(index=0; index<102400; index++)
	{
		pStatistics = kv_index_key_ptr(statistics_kv, index);
		if(pStatistics == NULL)
			break;

		if(_thread_statistics_info_is_display_msg_id(display_msg_id_table, pStatistics->msg_id) == dave_false)
		{
			if(frist_output == dave_false)
			{
				msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index, "\n\n");
			}
			frist_output = dave_false;

			msg_index += _thread_statistics_info_the_msg_id(statistics_kv, pStatistics->msg_id, &msg_ptr[msg_index], msg_len-msg_index);
		}
	}

	return msg_index;
}

// =====================================================================

ub
thread_statistics_info_api(void *statistics_kv, s8 *msg_ptr, ub msg_len)
{
	ub msg_index;

	msg_index = 0;

	if(statistics_kv == NULL)
	{
		msg_index += dave_snprintf(&msg_ptr[msg_index], msg_len-msg_index, "THREAD STATISTICS DISABLE!");
	}
	else
	{
		msg_index += _thread_statistics_info_api(statistics_kv, msg_ptr, msg_len);
	}

	return msg_index;
}

#endif


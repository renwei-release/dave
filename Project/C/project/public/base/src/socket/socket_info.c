/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "socket_core.h"

static ub
_socket_info_copy_digth(s8 *digit_ptr, ub digit_len, s8 *cmd)
{
	ub index;

	index = 0;
	digit_len -= 1;

	while((cmd[index] != '\0') && (index < digit_len))
	{
		if(t_is_digit((u8)cmd[index]) == dave_false)
		{
			break;
		}

		digit_ptr[index] = cmd[index];
		index ++;
	}

	return index;
}

// =====================================================================

ub
socket_info(s8 *info_ptr, ub info_len, s8 *cmd)
{
	s8 page_id_str[128];
	s8 *page_thread;
	ub cmd_next_index;
	ub page_id;

	cmd_next_index = _socket_info_copy_digth(page_id_str, sizeof(page_id_str), cmd);
	page_id = stringdigital(page_id_str);
	if(cmd[cmd_next_index] == '\0')
	{
		page_thread = NULL;
	}
	else
	{
		page_thread = &cmd[cmd_next_index];
		if(thread_id(page_thread) == INVALID_THREAD_ID)
		{
			page_thread = NULL;
		}
	}

	return socket_core_info(info_ptr, info_len, page_id, page_thread);
}

#endif


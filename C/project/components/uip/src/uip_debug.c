/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "uip_server.h"
#include "uip_server_monitor.h"

static ub
_uip_debug_info(s8 *info_ptr, ub info_len)
{
	ub info_index = 0;

	info_index += uip_server_info(&info_ptr[info_index], info_len-info_index);

	return info_index;
}

static ub
_uip_debug_setup_monitor_time(s8 *time_str, s8 *info_ptr, ub info_len)
{
	ub time = stringdigital(time_str);
	ub min_setup_time = (8 * 1000 * 1000);
	ub info_index = 0;

	if(time <= min_setup_time)
	{
		info_index = dave_snprintf(&info_ptr[info_index], info_len-info_index,
			"The set time %d cannot be less than %d, the setting is unsuccessful.",
			time, min_setup_time);
	}
	else
	{
		uip_server_monitor_time_consuming(time);

		info_index = dave_snprintf(&info_ptr[info_index], info_len-info_index,
			"The monitoring time %d is set successfully.",
			time);
	}

	return info_index;
}

// =====================================================================

void
uip_debug(ThreadId src, DebugReq *pReq)
{
	DebugRsp *pRsp = thread_reset_msg(pRsp);

	switch(pReq->msg[0])
	{
		case 'i':
				_uip_debug_info(pRsp->msg, sizeof(pRsp->msg));
			break;
		case 't':
				_uip_debug_setup_monitor_time(&pReq->msg[1], pRsp->msg, sizeof(pRsp->msg));
			break;
		default:
			break;
	}

	pRsp->ptr = pReq->ptr;

	write_msg(src, MSGID_DEBUG_RSP, pRsp);
}


/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_tools.h"
#include "dave_os.h"
#include "base_rxtx.h"
#include "base_tools.h"
#include "rxtx_confirm_transfer.h"
#include "rxtx_secure.h"
#include "rxtx_param.h"
#include "rxtx_tools.h"
#include "rxtx_log.h"


// =====================================================================

dave_bool
rxtx_system_rx(RXTX *pRxTx, ORDER_CODE order_id, ub frame_len, u8 *frame)
{
	dave_bool user_order_flag = dave_false;

	switch(order_id)
	{
		case ORDER_CODE_SUPPORT_NO_CRC:
				pRxTx->enable_data_crc = dave_false;
				RTTRACE("thread:%s port:%d disable crc!",
					thread_name(pRxTx->owner_thread), pRxTx->port);
			break;
		default:
				user_order_flag = dave_true;
			break;
	}

	return user_order_flag;
}

void
rxtx_system_no_crc_tx(RXTX *pRxTx)
{
	MBUF *data = dave_mmalloc(1024);

	data->tot_len = data->len = dave_snprintf(
		dave_mptr(data), data->len,
		"%d/%s", pRxTx->port, thread_name(pRxTx->owner_thread));

	rxtx_write(pRxTx->socket, ORDER_CODE_SUPPORT_NO_CRC, data);
}

#endif


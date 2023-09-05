/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "uip_client_send.h"
#include "uip_parsing.h"
#include "uip_log.h"

// =====================================================================

void
uip_client_init(MSGBODY *pMsg)
{

}

void
uip_client_main(MSGBODY *pMsg)
{
	switch((sb)(pMsg->msg_id))
	{
		case UIP_DATA_SEND_REQ:
				uip_client_send(pMsg->msg_src, (UIPDataSendReq *)(pMsg->msg_body));
			break;
		default:
			break;
	}
}

void
uip_client_exit(MSGBODY *pMsg)
{

}


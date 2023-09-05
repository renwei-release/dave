/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "dave_bdata.h"
#include "uip_server_register.h"
#include "uip_server_http.h"
#include "uip_server_monitor.h"
#include "uip_parsing.h"
#include "uip_tools.h"
#include "uip_log.h"

#define CFG_UIP_REPORT_TYPE "UIPReportType"

typedef enum {
	UIPReportType_disable,
	UIPReportType_head,
	UIPReportType_all
} UIPReportType;

static UIPReportType _uip_report_type = UIPReportType_head;

static void
_uip_server_report_reset(void)
{
	s8 cfg_data[1024];

	cfg_get_by_default(CFG_UIP_REPORT_TYPE, cfg_data, sizeof(cfg_data), "head");

	if(dave_strcmp(cfg_data, "disable") == dave_true)
	{
		_uip_report_type = UIPReportType_disable;
	}
	else if(dave_strcmp(cfg_data, "head") == dave_true)
	{
		_uip_report_type = UIPReportType_head;
	}
	else if(dave_strcmp(cfg_data, "all") == dave_true)
	{
		_uip_report_type = UIPReportType_all;
	}
	else
	{
		_uip_report_type = UIPReportType_head;
	}
}

static void
_uip_server_report_update(MSGBODY *msg)
{
	CFGUpdate *pUpdate = (CFGUpdate *)(msg->msg_body);

	if(dave_strcmp(pUpdate->cfg_name, CFG_UIP_REPORT_TYPE) == dave_true)
	{
		_uip_server_report_reset();
	}
}

static inline void
_uip_server_report(UIPStack *pRecvStack, UIPStack *pSendStack)
{
	dave_bool encode_body;
	void *recv;
	void *send;
	void *json;
	ub time_consuming = pSendStack->head.current_milliseconds - pRecvStack->head.current_milliseconds;

	encode_body = _uip_report_type == UIPReportType_all ? dave_true : dave_false;

	recv = uip_encode(pRecvStack, encode_body);
	send = uip_encode(pSendStack, encode_body);

	json = dave_json_malloc();
	dave_json_add_ub(json, "TIME-CONSUMING", time_consuming);
	dave_json_add_object(json, "recv", recv);
	dave_json_add_object(json, "send", send);

	BDATAJSON("UIP", json);
}

// =====================================================================

void
uip_server_report_init(void)
{
	_uip_server_report_reset();

	reg_msg(MSGID_CFG_UPDATE, _uip_server_report_update);
}

void
uip_server_report_exit(void)
{
	unreg_msg(MSGID_CFG_UPDATE);
}

void
uip_server_report(UIPStack *pRecvStack, UIPStack *pSendStack)
{
	if(_uip_report_type == UIPReportType_disable)
	{
		return;
	}

	_uip_server_report(pRecvStack, pSendStack);
}


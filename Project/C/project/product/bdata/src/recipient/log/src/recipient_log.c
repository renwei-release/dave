/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_BDATA__
#include "dave_define.h"
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "recorder_file.h"
#include "bdata_msg.h"

static void
_recipient_log_data(void *pNote, BDataLogReq *pReq)
{
	s8 date_str[128], mac_str[128], ipv4_str[128], ipv6_str[128];
	ub date_len, mac_len, ipv4_len, ipv6_len;

	date_len = dave_snprintf(date_str, sizeof(date_str), "%04d.%02d.%02d %02d:%02d:%02d",
		pReq->local_date.year, pReq->local_date.month, pReq->local_date.day,
		pReq->local_date.hour, pReq->local_date.minute, pReq->local_date.second);

	mac_len = dave_snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
		pReq->host_mac[0], pReq->host_mac[1], pReq->host_mac[2],
		pReq->host_mac[3], pReq->host_mac[4], pReq->host_mac[5]);

	ipv4_len = dave_snprintf(ipv4_str, sizeof(ipv4_str), "%d.%d.%d.%d",
		pReq->host_ipv4[0], pReq->host_ipv4[1], pReq->host_ipv4[2], pReq->host_ipv4[3]);

	ipv6_len = dave_snprintf(ipv6_str, sizeof(ipv6_str), "%02d%02d.%02d%02d.%02d%02d.%02d%02d.%02d%02d.%02d%02d.%02d%02d.%02d%02d",
		pReq->host_ipv6[0], pReq->host_ipv6[1], pReq->host_ipv6[2], pReq->host_ipv6[3],
		pReq->host_ipv6[4], pReq->host_ipv6[5], pReq->host_ipv6[6], pReq->host_ipv6[7],
		pReq->host_ipv6[8], pReq->host_ipv6[9], pReq->host_ipv6[10], pReq->host_ipv6[11],
		pReq->host_ipv6[12], pReq->host_ipv6[13], pReq->host_ipv6[14], pReq->host_ipv6[15]);

	recorder_file_str(pNote, "version", pReq->version, dave_strlen(pReq->version));
	recorder_file_str(pNote, "sub_flag", pReq->sub_flag, dave_strlen(pReq->sub_flag));
	recorder_file_str(pNote, "local_date", date_str, date_len);

	recorder_file_str(pNote, "host", pReq->host_name, dave_strlen(pReq->host_name));
	recorder_file_str(pNote, "mac", mac_str, mac_len);
	recorder_file_str(pNote, "ipv4", ipv4_str, ipv4_len);
	recorder_file_str(pNote, "ipv6", ipv6_str, ipv6_len);

	recorder_file_str(pNote, "log", dave_mptr(pReq->log_data), dave_mlen(pReq->log_data));
}

static RetCode
_recipient_log(BDataLogReq *pReq)
{
	s8 product[DAVE_VERNO_STR_LEN];
	void *pNote;

	pNote = recorder_file_open(dave_verno_product(pReq->version, product, sizeof(product)));
	if(pNote == NULL)
		return RetCode_store_data_failed;

	_recipient_log_data(pNote, pReq);

	recorder_file_close(pNote);

	return RetCode_OK;
}

// =====================================================================

void
recipient_log(ThreadId src, BDataLogReq *pReq)
{
	BDataLogRsp *pRsp = thread_msg(pRsp);

	pRsp->ret = _recipient_log(pReq);
	pRsp->ptr = pReq->ptr;

	id_msg(src, BDATA_LOG_RSP, pRsp);

	dave_mfree(pReq->log_data);
}

#endif


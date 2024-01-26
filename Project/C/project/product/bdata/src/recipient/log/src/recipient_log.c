/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_BDATA__
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "recorder_file.h"
#include "recorder_aliyun.h"
#include "bdata_msg.h"
#include "bdata_log.h"

static inline void
_recipient_log_data(void *pNote, BDataLogReq *pReq)
{
	s8 date_str[64], mac_str[64], ipv4_str[32], ipv6_str[64], line_str[32];
	ub date_len, mac_len, ipv4_len, ipv6_len, line_len;

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

	line_len = dave_snprintf(line_str, sizeof(line_str), "%d", pReq->line);

	recorder_file_str(pNote, "version", pReq->version, dave_strlen(pReq->version));
	recorder_file_str(pNote, "sub_flag", pReq->sub_flag, dave_strlen(pReq->sub_flag));
	recorder_file_str(pNote, "local_date", date_str, date_len);

	recorder_file_str(pNote, "fun", pReq->fun, dave_strlen(pReq->fun));
	recorder_file_str(pNote, "line", line_str, line_len);

	recorder_file_str(pNote, "host", pReq->host_name, dave_strlen(pReq->host_name));
	recorder_file_str(pNote, "mac", mac_str, mac_len);
	recorder_file_str(pNote, "ipv4", ipv4_str, ipv4_len);
	recorder_file_str(pNote, "ipv6", ipv6_str, ipv6_len);

	recorder_file_obj(pNote, "log", ms8(pReq->log_data), mlen(pReq->log_data));
}

static inline s8 *
_recipient_log_file(s8 *file_ptr, ub file_len, BDataLogReq *pReq)
{
	s8 product[DAVE_VERNO_STR_LEN];

	if(pReq->sub_flag[0] == '\0')
	{
		dave_product(pReq->version, file_ptr, file_len);
	}
	else
	{
		dave_product(pReq->version, product, sizeof(product));
		dave_snprintf(file_ptr, file_len, "%s/%s", product, pReq->sub_flag);
	}

	return file_ptr;
}

static inline void
_recipient_log_aliyun(s8 *log_file, void *pNote)
{
	void *pJson;

	pJson = recorder_file_json(pNote);

	if(aliyun_log_json(log_file, pJson) == dave_false)
	{
		BDLOG("log_file save failed!", log_file);
	}
}

static inline RetCode
_recipient_log(BDataLogReq *pReq)
{
	s8 file_name[1024];
	void *pNote;

	pNote = recorder_file_open(_recipient_log_file(file_name, sizeof(file_name), pReq));
	if(pNote == NULL)
	{
		BDLOG("version:%s <%s:%d> store %s failed!",
			pReq->version, pReq->fun, pReq->line,
			file_name);
		return RetCode_store_data_failed;
	}

	BDTRACE("pNote:%lx file_name:%s sub_flag:%s version:%s <%s:%d>",
		pNote,
		file_name, pReq->sub_flag,
		pReq->version, pReq->fun, pReq->line);

	_recipient_log_data(pNote, pReq);

	_recipient_log_aliyun(pReq->sub_flag, pNote);

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


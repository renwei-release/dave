/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_BDATA_H__
#define __DAVE_BDATA_H__
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "dave_os.h"
#include "bdata_msg.h"
#undef vsnprintf
#include <stdio.h>
#include <stdlib.h>

#define __BDATABASE__(sub, pReq) {\
	dave_strcpy(pReq->version, dave_verno(), sizeof(pReq->version));\
	dave_strcpy(pReq->sub_flag, sub, sizeof(pReq->sub_flag));\
	t_time_get_date(&(pReq->local_date));\
\
	dave_strcpy(pReq->fun, __func__, sizeof(pReq->fun));\
	pReq->line = __LINE__;\
\
	dave_os_load_host_name(pReq->host_name, sizeof(pReq->host_name));\
	dave_os_load_mac(pReq->host_mac);\
	dave_os_load_ip(pReq->host_ipv4, pReq->host_ipv6);\
\
	pReq->ptr = pReq;\
}

/*
 * Call example:
 * BDATALOG("my log flag xxxx", "%s", str_data);
 */
#define BDATALOG(sub, log, ...) {\
	BDataLogReq *pReq = thread_reset_msg(pReq);\
\
	pReq->level = BDataLogLevel_normal;\
\
	__BDATABASE__(sub, pReq)\
	pReq->log_data = dave_mmalloc(1024 * 32);\
	pReq->log_data->tot_len = pReq->log_data->len = dave_snprintf(dave_mptr(pReq->log_data), dave_mlen(pReq->log_data), log, ##__VA_ARGS__);\
\
	name_msg(BDATA_THREAD_NAME, BDATA_LOG_REQ, pReq);\
}

/*
 * Call example:
 * BDATAJSON("my log flag xxxx", json);
 */
#define BDATAJSON(sub, json) {\
\
	if(json != NULL)\
	{\
		BDataLogReq *pReq = thread_reset_msg(pReq);\
\
		pReq->level = BDataLogLevel_normal;\
\
		__BDATABASE__(sub, pReq)\
		pReq->log_data = dave_json_to_mbuf(json);\
\
		name_msg(BDATA_THREAD_NAME, BDATA_LOG_REQ, pReq);\
\
		dave_json_free(json);\
	}\
}

/*
 * Call example:
 * BDATARPT("my log flag xxxx", "%s", str_data);
 */
#define BDATARPT(sub, log, ...) {\
	BDataLogReq *pReq = thread_reset_msg(pReq);\
\
	pReq->level = BDataLogLevel_report;\
\
	__BDATABASE__(sub, pReq)\
	pReq->log_data = dave_mmalloc(1024 * 32);\
	pReq->log_data->tot_len = pReq->log_data->len = dave_snprintf(dave_mptr(pReq->log_data), dave_mlen(pReq->log_data), log, ##__VA_ARGS__);\
\
	name_msg(BDATA_THREAD_NAME, BDATA_LOG_REQ, pReq);\
}

#endif


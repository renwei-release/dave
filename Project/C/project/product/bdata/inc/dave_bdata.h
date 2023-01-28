/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_BDATA_H__
#define __DAVE_BDATA_H__
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "bdata_msg.h"
#undef vsnprintf
#include <stdio.h>
#include <stdlib.h>

#define __BDATABASE__(sub, pReq) {\
	dave_strcpy(pReq->version, dave_verno(), sizeof(pReq->version));\
	dave_strcpy(pReq->sub_flag, sub, sizeof(pReq->sub_flag));\
	t_time_get_date(&(pReq->local_date));\
	dave_os_load_host_name(pReq->host_name, sizeof(pReq->host_name));\
	dave_os_load_mac(pReq->host_mac);\
	dave_os_load_ip(pReq->host_ipv4, pReq->host_ipv6);\
}

/*
 * Call example:
 * BDATALOG("my log flag xxxx", "%s", str_data);
 */
#define BDATALOG(sub, log, ...) {\
	BDataLogReq *pReq = thread_msg(pReq);\
\
	__BDATABASE__(sub, pReq)\
	pReq->log_data = dave_mmalloc(1024 * 32);\
	pReq->log_data->tot_len = pReq->log_data->len = dave_snprintf(dave_mptr(pReq->log_data), dave_mlen(pReq->log_data), log, ##__VA_ARGS__);\
	pReq->ptr = pReq;\
\
	name_msg(BDATA_THREAD_NAME, BDATA_LOG_REQ, pReq);\
}

/*
 * Call example:
 * BDATAJSON("my log flag xxxx", json);
 */
#define BDATAJSON(sub, json) {\
	BDataLogReq *pReq;\
\
	if(json != NULL)\
	{\
		pReq = thread_msg(pReq);\
\
		__BDATABASE__(sub, pReq)\
		pReq->log_data = dave_json_to_mbuf(json);\
		pReq->ptr = pReq;\
\
		name_msg(BDATA_THREAD_NAME, BDATA_LOG_REQ, pReq);\
\
		dave_json_free(json);\
	}\
}

#endif


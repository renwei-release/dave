/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_BDATA__
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "dave_os.h"
#include "dave_verno.h"
#include "dave_email.h"
#include "bdata_msg.h"
#include "bdata_log.h"

static void
_reporter_subject(
	s8 *subject_ptr, ub subject_len,
	ThreadId src)
{
	dave_snprintf(subject_ptr, subject_len,
		"BDATA %s",
		thread_name(src));
}

static void
_reporter_from(s8 *from_ptr, ub from_len, BDataLogReq *pReq)
{
	dave_snprintf(from_ptr, from_len,
		"%s/%s/%s",
		macstr(pReq->host_mac),
		pReq->host_name,
		ipv4str(pReq->host_ipv4, 0));
}

static void
_reporter_to(s8 *to_ptr, ub to_len)
{
	u8 mac[DAVE_MAC_ADDR_LEN];
	s8 hostname[128];
	u8 ip_v4[DAVE_IP_V4_ADDR_LEN];
	u8 ip_v6[DAVE_IP_V6_ADDR_LEN];

	dave_os_load_mac(mac);
	dave_os_load_host_name(hostname, sizeof(hostname));
	dave_os_load_ip(ip_v4, ip_v6);

	dave_snprintf(to_ptr, to_len,
		"%s/%s/%s",
		macstr(mac),
		hostname,
		ipv4str(ip_v4, 0));
}

static ub
_reporter_body(
	s8 *body_ptr, ub body_len,
	BDataLogReq *pReq)
{
	s8 from[256], to[256];

	_reporter_from(from, sizeof(from), pReq);
	_reporter_to(to, sizeof(to));

	return dave_snprintf(body_ptr, body_len,
"\r\n%s\r\n"\
"\r\n\r\n%s -> %s\r\n"\
"level:%s\r\n"\
"version:%s\r\n"\
"sub_flag:%s\r\n"\
"local_date:%s\r\n"\
"\r\n===================================\r\n"\
"FROM:%s\r\n\r\n",

		ms8(pReq->log_data),

		from, to,

		t_auto_BDataLogLevel_str(pReq->level),
		pReq->version,
		pReq->sub_flag,
		t_a2b_date_str(&(pReq->local_date)),

		dave_verno());
}

static void
_reporter_email(s8 *subject, MBUF *body)
{
	EmailSendReq *pReq = thread_msg(pReq);

	dave_strcpy(pReq->subject, subject, sizeof(pReq->subject));
	pReq->content = body;
	pReq->ptr = body;

	name_msg(EMAIL_THREAD_NAME, EMAIL_SEND_REQ, pReq);
}

static void
_reporter(ThreadId src, BDataLogReq *pReq)
{
	s8 subject[128];
	ub body_len = 1024 * 8;
	MBUF *body_ptr = dave_mmalloc(body_len);

	_reporter_subject(subject, sizeof(subject), src);

	body_ptr->tot_len = body_ptr->len = _reporter_body(ms8(body_ptr), body_len, pReq);

	_reporter_email(subject, body_ptr);
}

// =====================================================================

void
reporter(ThreadId src, BDataLogReq *pReq)
{
	if(pReq->level != BDataLogLevel_report)
		return;

	BDLOG("%s %s %s", thread_name(src), pReq->version, pReq->sub_flag);

	_reporter(src, pReq);
}

#endif


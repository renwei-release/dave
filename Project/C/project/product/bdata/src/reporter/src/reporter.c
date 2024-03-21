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
#include "bdata_msg.h"
#include "bdata_log.h"

#define CFG_EMAIL_USERNAME "EmailUsername"
#define CFG_EMAIL_PASSWORD "EmailPassword"
#define CFG_EMAIL_SMTP_URL "EmailSmtpUrl"
#define CFG_EMAIL_FROM	"EmailFrom"

static void
_reporter_param(
	s8 *username_ptr, ub username_len,
	s8 *password_ptr, ub password_len,
	s8 *smtp_url_ptr, ub smtp_url_len,
	s8 *from_email_ptr, ub from_email_len,
	s8 *to_email_ptr, ub to_email_len)
{
	cfg_get_by_default(CFG_EMAIL_USERNAME, username_ptr, username_len, "renwei_msn@hotmail.com");
	cfg_get_by_default(CFG_EMAIL_PASSWORD, password_ptr, password_len, "");
	cfg_get_by_default(CFG_EMAIL_SMTP_URL, smtp_url_ptr, smtp_url_len, "smtp://smtp-mail.outlook.com:587");

	cfg_get_by_default(CFG_EMAIL_FROM, from_email_ptr, from_email_len, "renwei_msn@hotmail.com");

	dave_snprintf(to_email_ptr, to_email_len, "528.ww@163.com");
}

static void
_reporter_subject(
	s8 *subject_ptr, ub subject_len,
	BDataLogReq *pReq)
{
	dave_snprintf(subject_ptr, subject_len,
		"BDATA %s",
		pReq->version);
}

static void
_reporter_body(
	s8 *body_ptr, ub body_len,
	BDataLogReq *pReq)
{
	dave_snprintf(body_ptr, body_len,
"level:%s\r\n"\
"version:%s\r\n"\
"sub_flag:%s\r\n"\
"local_date:%s\r\n",
		t_auto_BDataLogLevel_str(pReq->level),
		pReq->version,
		pReq->sub_flag,
		t_a2b_date_str(&(pReq->local_date)));
}

static void
_reporter(ThreadId src, BDataLogReq *pReq)
{
	s8 username[128];
	s8 password[128];
	s8 smtp_url[128];
	s8 from_email[128];
	s8 to_email[128];
	s8 subject[128];
	ub body_len = 1024 * 8;
	s8 *body_ptr = dave_malloc(body_len);

	_reporter_param(
		username, sizeof(username),
		password, sizeof(password),
		smtp_url, sizeof(smtp_url),
		from_email, sizeof(from_email),
		to_email, sizeof(to_email));

	if((dave_strlen(username) == 0)
		|| (dave_strlen(password) == 0)
		|| (dave_strlen(smtp_url) == 0)
		|| (dave_strlen(from_email) == 0))
	{
		BDLOG("can't find account, username:%s password:%s smtp_url:%s from_email:%s",
			username, password, smtp_url, from_email);
		return;
	}

	_reporter_subject(subject, sizeof(subject), pReq);

	_reporter_body(body_ptr, body_len, pReq);

	dave_curl_email(username, password, smtp_url, from_email, to_email, subject, body_ptr);

	dave_free(body_ptr);
}

// =====================================================================

void
reporter(ThreadId src, BDataLogReq *pReq)
{
	if(pReq->level != BDataLogLevel_report)
		return;

	BDLOG("%s", thread_name(src));

	_reporter(src, pReq);
}

#endif


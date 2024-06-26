/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_email.h"
#include "dave_os.h"
#include "dave_echo.h"
#include "dave_3rdparty.h"
#include "email_log.h"

#define CFG_EMAIL_SMTP_URL "EmailSmtpUrl"
#define CFG_EMAIL_USERPWD "EmailUserPwd"
#define CFG_EMAIL_FROM	"EmailFrom"
#define CFG_EMAIL_TO "EmailTo"

static void
_email_param(
	s8 *smtp_url_ptr, ub smtp_url_len,
	s8 *userpwd_ptr, ub userpwd_len,
	s8 *from_email_ptr, ub from_email_len,
	s8 *to_email_ptr, ub to_email_len)
{
	cfg_get_by_default(CFG_EMAIL_SMTP_URL, smtp_url_ptr, smtp_url_len, "");
	cfg_get_by_default(CFG_EMAIL_USERPWD, userpwd_ptr, userpwd_len, "");
	cfg_get_by_default(CFG_EMAIL_FROM, from_email_ptr, from_email_len, "");
	cfg_get_by_default(CFG_EMAIL_TO, to_email_ptr, to_email_len, "[]");
}

static RetCode
_email_send(
	s8 *smtp_url, s8 *userpwd,
	s8 *from_email, s8 *to_email,
	s8 *subject, s8 *context,
	s8 *attachment)
{
	dave_bool ret;

	ret = dave_quickmail(userpwd, "", smtp_url, from_email, to_email, subject, context, attachment);
	if(ret == dave_false)
	{
		EMAILLOG("send failed! smtp:%s userpwd:%s from:%s to:%s subject:%s", smtp_url, userpwd, from_email, to_email, subject);
	}
	else
	{
		EMAILLOG("from:%s to:%s subject:%s send success!", from_email, to_email, subject);
	}

	if(ret == dave_true)
		return RetCode_OK;
	else
		return RetCode_Send_failed;
}

// =====================================================================

RetCode
email_send(s8 *subject, s8 *context, s8 *attachment)
{
	s8 smtp_url[128];
	s8 userpwd[128];
	s8 from_email[128];
	s8 to_email[2048];

	_email_param(smtp_url, sizeof(smtp_url), userpwd, sizeof(userpwd), from_email, sizeof(from_email), to_email, sizeof(to_email));

	if((dave_strlen(smtp_url) == 0)
		|| (dave_strlen(userpwd) == 0)
		|| (dave_strlen(from_email) == 0))
	{
		EMAILLOG("can't find account, smtp_url:%s username:%s from_email:%s",
			smtp_url, userpwd, from_email);
		return RetCode_invalid_account;
	}

	return _email_send(smtp_url, userpwd, from_email, to_email, subject, context, attachment);
}


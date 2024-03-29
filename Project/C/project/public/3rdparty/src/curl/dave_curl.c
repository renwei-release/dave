/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(CURL_3RDPARTY)
#include "dave_base.h"
#include "dave_tools.h"
#include "party_log.h"
#include "curl.h"

typedef struct {
	s8 *smtp_snd_ptr;
	ub smtp_snd_index;
	ub smtp_snd_len;
} CurlSMTPSend;

static size_t
_curl_smtp_write_func(void *ptr, size_t size, size_t nmemb, FILE *pCurl)
{
	PARTYLOG("size:%d nmemb:%d", size, nmemb);

	return size * nmemb;
}

static size_t
_curl_smtp_read_func(void *ptr, size_t size, size_t nmemb, FILE *pCurl)
{
	CurlSMTPSend *smtp_send = (CurlSMTPSend *)pCurl;
	size_t read_length;

	if((size == 0) || (nmemb == 0) || ((size * nmemb) < 1))
	{
		return 0;
	}

	if((smtp_send == NULL) || (smtp_send->smtp_snd_ptr == NULL) || (smtp_send->smtp_snd_index >= smtp_send->smtp_snd_len))
	{
		return 0;
	}
	else
	{
		read_length = size * nmemb;

		if(read_length > (smtp_send->smtp_snd_len - smtp_send->smtp_snd_index))
		{
			read_length = smtp_send->smtp_snd_len - smtp_send->smtp_snd_index;
		}

		dave_memcpy(ptr, &(smtp_send->smtp_snd_ptr[smtp_send->smtp_snd_index]), read_length);

		smtp_send->smtp_snd_index += read_length;

		return read_length;
	}
}

// =====================================================================

dave_bool
dave_curl_email(s8 *username, s8 *password, s8 *smtp_url, s8 *from_email, s8 *to_email, s8 *subject, s8 *body)
{
	CURL *curl;
	CURLcode res = CURLE_OK;
	ub timer_out = 360;

	curl = curl_easy_init();
 	if(curl)
	{
		struct curl_slist *recipients = NULL;
		CurlSMTPSend *smtp_send = dave_ralloc(sizeof(CurlSMTPSend));

		recipients = curl_slist_append(recipients, to_email);

		if(dave_strlen(password) != 0)
		{
			curl_easy_setopt(curl, CURLOPT_USERNAME, username);
			curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
		}
		else
		{
			curl_easy_setopt(curl, CURLOPT_USERPWD, username);
		}

		curl_easy_setopt(curl, CURLOPT_URL, smtp_url);
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from_email);
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

		smtp_send->smtp_snd_len = dave_strlen(subject) + dave_strlen(body) + 1024;
		smtp_send->smtp_snd_ptr = dave_malloc(smtp_send->smtp_snd_len);
		smtp_send->smtp_snd_len = dave_snprintf(smtp_send->smtp_snd_ptr, smtp_send->smtp_snd_len,
			"From: \"%s\"\r\nTo: \"%s\"\r\nSubject: %s\r\n\r\n%s\r\n",
			from_email, to_email, subject, body);
		smtp_send->smtp_snd_index = 0;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)smtp_send);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _curl_smtp_write_func);
		curl_easy_setopt(curl, CURLOPT_READDATA, (void *)smtp_send);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, _curl_smtp_read_func);

		curl_easy_setopt(curl, CURLOPT_TIMEOUT, timer_out);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timer_out);
		curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
		curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_DEFAULT);
		curl_easy_setopt(curl, CURLOPT_SSL_SESSIONID_CACHE, 0L); 
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);

		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
		{
			PARTYLOG("failed:%s username:%s password:%s smtp_url:%s from_email:%s to_email:%s",
				curl_easy_strerror(res),
				username, password, smtp_url, from_email, to_email);
		}

		curl_slist_free_all(recipients);
		curl_easy_cleanup(curl);

		dave_free(smtp_send->smtp_snd_ptr);
		dave_free(smtp_send);
	}

	if(res != CURLE_OK)
		return dave_false;
	else
		return dave_true;
}

#endif


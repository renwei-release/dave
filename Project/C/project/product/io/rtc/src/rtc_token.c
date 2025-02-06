/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_tools.h"
#include "dave_base.h"
#include "dave_os.h"
#include "dave_rtc.h"
#include "tlv_parse.h"
#include "rtc_param.h"
#include "rtc_token.h"
#include "rtc_log.h"

void *_rtc_token_kv = NULL;

static void
_rtc_token_build(s8 *token_ptr, ub token_len)
{
	u8 mac[DAVE_MAC_ADDR_LEN];
	s8 token_string[256];
	s8 md5_str_1[DAVE_MD5_HASH_STR_LEN];
	s8 md5_str_2[DAVE_MD5_HASH_STR_LEN];

	dave_os_load_mac(mac);

	dave_snprintf(token_string, sizeof(token_string), "%s%ld%ld", macstr(mac), t_rand(), dave_os_time_ns());
	t_crypto_md5_str(md5_str_1, (u8 *)token_string, dave_strlen(token_string));

	dave_snprintf(token_string, sizeof(token_string), "%s%ld%ld", macstr(mac), dave_os_time_ns(), t_rand());
	t_crypto_md5_str(md5_str_2, (u8 *)token_string, dave_strlen(token_string));

	dave_snprintf(token_ptr, token_len, "%s%s", md5_str_1, md5_str_2);
}

static RTCToken *
_rtc_token_malloc(ThreadId src_id, s8 *src_gid, s8 *src_name, s8 *terminal_type, s8 *terminal_id)
{
	RTCToken *pToken = dave_ralloc(sizeof(RTCToken));
	s8 kv_name[128];

	_rtc_token_build(pToken->token, sizeof(pToken->token));
	dave_strcpy(pToken->terminal_type, terminal_type, sizeof(pToken->terminal_type));
	dave_strcpy(pToken->terminal_id, terminal_id, sizeof(pToken->terminal_id));

	pToken->src_id = src_id;
	dave_strcpy(pToken->src_gid, src_gid, sizeof(pToken->src_gid));
	dave_strcpy(pToken->src_name, src_name, sizeof(pToken->src_name));

	pToken->data_length = 0;
	pToken->recv_serial = pToken->send_serial = pToken->local_serial = 0;
	dave_snprintf(kv_name, sizeof(kv_name), "tokenkv-%lx", pToken);
	pToken->pre_buffer_kv = kv_malloc(kv_name, 0, NULL);

	pToken->pClient = NULL;

	return pToken;
}

static RetCode
_rtc_token_buffer_recycle(void *ramkv, s8 *key)
{
	MBUF *pre_buffer = (MBUF *)kv_del_key_ptr(ramkv, key);

	if(pre_buffer == NULL)
		return RetCode_not_my_data;

	dave_mfree(pre_buffer);

	return RetCode_OK;
}

static void
_rtc_token_delay_free(TIMERID timer_id, ub thread_index, void *param)
{
	RTCToken *pToken = (RTCToken *)param;

	if(pToken->pre_buffer_kv != NULL)
	{
		kv_free(pToken->pre_buffer_kv, _rtc_token_buffer_recycle);
	}

	dave_free(pToken);
}

static void
_rtc_token_free(RTCToken *pToken)
{
	s8 timer_name[128];

	if(pToken != NULL)
	{
		dave_snprintf(timer_name, sizeof(timer_name), "tokenfree-%lx", pToken);

		base_timer_param_creat(timer_name, _rtc_token_delay_free, pToken, sizeof(void *), 3000);
	}
}

static RetCode
_rtc_token_recycle(void *ramkv, s8 *key)
{
	RTCToken *pToken = kv_del_key_ptr(ramkv, key);

	if(pToken == NULL)
		return RetCode_empty_data;

	_rtc_token_free(pToken);

	return RetCode_OK;
}

static void
_rtc_token_timer_out(void *ramkv, s8 *key)
{
	RTCToken *pToken = kv_inq_key_ptr(ramkv, key);

	if(pToken != NULL)
	{
		if((pToken->lift_counter --) <= 0)
		{
			rtc_token_del(pToken->token);
		}
	}
}

// =====================================================================

void
rtc_token_init(void)
{
    _rtc_token_kv = kv_malloc("rtctoken", 30, _rtc_token_timer_out);
}

void
rtc_token_exit(void)
{
    kv_free(_rtc_token_kv, _rtc_token_recycle);
}

s8 *
rtc_token_add(ThreadId src_id, s8 *src_gid, s8 *src_name, s8 *terminal_type, s8 *terminal_id)
{
	RTCToken *pToken = _rtc_token_malloc(src_id, src_gid, src_name, terminal_type, terminal_id);

	kv_add_key_ptr(_rtc_token_kv, pToken->token, pToken);

	return pToken->token;
}

RTCToken *
rtc_token_inq(s8 *token)
{
	RTCToken *pToken = (RTCToken *)kv_inq_key_ptr(_rtc_token_kv, token);

	if(pToken != NULL)
	{
		pToken->lift_counter = TOKEN_LIFT_MAX;
	}

	return pToken;
}

void
rtc_token_del(s8 *token)
{
    RTCToken *pToken = kv_del_key_ptr(_rtc_token_kv, token);

	_rtc_token_free(pToken);
}

void
rtc_token_add_client(s8 *token, RTCClient *pClient)
{
	RTCToken *pToken = rtc_token_inq(token);

	if(pToken == NULL)
	{
		RTCLOG("can't find pToken:%s", token);
		return;
	}

	RTCLOG("token:%s socket:%d", token, pClient->socket);

    pToken->pClient = pClient;
}

RTCClient *
rtc_token_inq_client(s8 *token)
{
    RTCToken *pToken = rtc_token_inq(token);

	if(pToken == NULL)
	{
		return NULL;
	}

	return pToken->pClient;
}

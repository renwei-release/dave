/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_osip.h"
#include "sip_signal.h"
#include "sip_client.h"
#include "sip_server.h"
#include "sip_reg.h"
#include "sip_state.h"
#include "sip_automatic.h"
#include "sip_global_lock.h"
#include "sip_call_class.h"
#include "sip_log.h"

static void *_sip_signal_socket_kv = NULL;

static osip_message_t *
_sip_signal_message_parse(const char *buf, size_t length)
{
	osip_message_t *sip;
	int ret;

	if((length >= SIP_RECV_BUFFER_MAX) || (length <= 4))
	{
		SIPLOG("invalid length:%d", length);
		return NULL;
	}

	osip_message_init(&sip);

	ret = osip_message_parse(sip, buf, length);
	if(ret != OSIP_SUCCESS)
	{
		SIPLOG("ret:%d data:%d/%s", ret, length, buf);
		return NULL;
	}

	return sip;
}

static int
_sip_signal_message_distribution(SIPSignal *pSignal, osip_message_t *sip)
{
	int status_code = 0;

	if((dave_strcmp(sip->cseq->method, "REGISTER") == dave_true)
		&& (pSignal->reg_recv_fun != NULL))
	{
		status_code = (int)pSignal->reg_recv_fun(pSignal->reg_recv_param, sip);
	}
	else if((dave_strcmp(sip->cseq->method, "INVITE") == dave_true)
		&& (pSignal->inv_recv_fun != NULL))
	{
		status_code = (int)pSignal->inv_recv_fun(pSignal->inv_recv_param, sip);
	}
	else if((dave_strcmp(sip->cseq->method, "BYE") == dave_true)
		&& (pSignal->bye_recv_fun != NULL))
	{
		status_code = (int)pSignal->bye_recv_fun(pSignal->bye_recv_param, sip);
	}
	else
	{
		SIPLOG("unprocess cseq:%s %s", sip->cseq->method, sip->cseq->number);
	}

	return status_code;
}

static ub
_sip_signal_find_sip_package_end(SIPSignal *pSignal)
{
	ub sip_package_end_index;
	ub sip_length, content_length;

	sip_package_end_index = osip_find_end_flag(&pSignal->recv_buffer[pSignal->recv_e_index], pSignal->recv_w_index - pSignal->recv_e_index);
	if(sip_package_end_index == 0)
	{
		/*
		 * At least 4 bytes of margin are required to prevent missed detection.
		 * \r\n\r\n
		 */
		if(pSignal->recv_w_index > 4)
			pSignal->recv_e_index = pSignal->recv_w_index - 4;
		else
			pSignal->recv_e_index = 0;
		return 0;
	}

	sip_package_end_index = pSignal->recv_e_index + sip_package_end_index;

	sip_length = sip_package_end_index - pSignal->recv_r_index;

	content_length = osip_content_length(&pSignal->recv_buffer[pSignal->recv_r_index], sip_length);

	if((pSignal->recv_w_index - pSignal->recv_r_index) >= (sip_length + content_length))
		return sip_package_end_index + content_length;

	return 0;
}

static dave_bool
_sip_signal_is_re_requeset(SIPSignal *pSignal, osip_message_t *sip)
{
	if(sip->sip_method == NULL)
		return dave_false;

	if(pSignal->register_request == sip)
		return dave_true;

	if(pSignal->invite_request == sip)
		return dave_true;

	if(pSignal->bye_request == sip)
		return dave_true;

	return dave_false;;
}

static void
_sip_signal_recv(SIPSignal *pSignal)
{
	ub sip_package_end_index;
	osip_message_t *sip;
	int status_code;

	sip_package_end_index = _sip_signal_find_sip_package_end(pSignal);
	if(sip_package_end_index == 0)
	{
		return;
	}

	sip = _sip_signal_message_parse(&pSignal->recv_buffer[pSignal->recv_r_index], sip_package_end_index - pSignal->recv_r_index);
	pSignal->recv_r_index = pSignal->recv_e_index = sip_package_end_index;

	if(sip == NULL)
	{
		SIPLOG("r:%d e:%d w:%d",
			pSignal->recv_r_index, pSignal->recv_e_index, pSignal->recv_w_index);
		return;
	}

	SIPLOG("r:%d e:%d w:%d version:%s method:%s status_code:%d reason_phrase:%s",
		pSignal->recv_r_index, pSignal->recv_e_index, pSignal->recv_w_index,
		sip->sip_version, sip->sip_method, sip->status_code, sip->reason_phrase);

	status_code = _sip_signal_message_distribution(pSignal, sip);

	sip_automatic_recv(pSignal, sip, status_code);

	osip_message_free(sip);
}

static void
_sip_signal_send(SIPSignal *pSignal, MBUF *pData)
{
	SocketWrite *pWrite = thread_msg(pWrite);

	pWrite->socket = pSignal->signal_socket;
	pWrite->IPInfo.protocol = IPProtocol_UDP;
	strip(pSignal->local_ip, 0, pWrite->IPInfo.src_ip, sizeof(pWrite->IPInfo.src_ip));
	pWrite->IPInfo.src_port = stringdigital(pSignal->local_port);
	strip(pSignal->server_ip, 0, pWrite->IPInfo.dst_ip, sizeof(pWrite->IPInfo.dst_ip));
	pWrite->IPInfo.dst_port = stringdigital(pSignal->server_port);
	pWrite->data_len = pData->tot_len;
	pWrite->data = pData;
	pWrite->close_flag = SOCKETINFO_SND;

	SIPLOG("%s->%s socket:%d data:%d",
		ipv4str(pWrite->IPInfo.src_ip, pWrite->IPInfo.src_port),
		ipv4str2(pWrite->IPInfo.dst_ip, pWrite->IPInfo.dst_port),
		pSignal->signal_socket,
		pWrite->data_len);

	name_msg(SOCKET_THREAD_NAME, SOCKET_WRITE, pWrite);
}

static RetCode
_sip_signal_call_recycle(void *ramkv, s8 *key)
{
	SIPCall *pCall = (SIPCall *)kv_del_key_ptr(ramkv, key);

	if(pCall == NULL)
	{
		return RetCode_empty_data;
	}

	sip_call_release((SIPSignal *)(pCall->signal), pCall);

	return RetCode_OK;
}

static void
_sip_signal_signal_creat(
	SIPSignal *pSignal,
	SIPSignalType signal_type,
	s32 signal_socket,
	s8 *server_ip, s8 *server_port, s8 *username, s8 *password,
	s8 *local_ip, s8 *local_port,
	s8 *rtp_ip, s8 *rtp_port)
{
	s8 kv_name[64];

	pSignal->signal_type = signal_type;
	pSignal->signal_socket = signal_socket;

	dave_strcpy(pSignal->server_ip, server_ip, sizeof(pSignal->server_ip));
	dave_strcpy(pSignal->server_port, server_port, sizeof(pSignal->server_port));
	dave_strcpy(pSignal->username, username, sizeof(pSignal->username));
	dave_strcpy(pSignal->password, password, sizeof(pSignal->password));	
	dave_strcpy(pSignal->local_ip, local_ip, sizeof(pSignal->local_ip));
	dave_strcpy(pSignal->local_port, local_port, sizeof(pSignal->local_port));
	dave_strcpy(pSignal->rtp_ip, rtp_ip, sizeof(pSignal->rtp_ip));
	dave_strcpy(pSignal->rtp_port, rtp_port, sizeof(pSignal->rtp_port));

	pSignal->recv_r_index = pSignal->recv_e_index = pSignal->recv_w_index = 0;

	pSignal->reg_recv_fun = pSignal->inv_recv_fun = pSignal->bye_recv_fun = NULL;
	pSignal->reg_recv_param = pSignal->inv_recv_param = pSignal->bye_recv_param = NULL;

	t_lock_reset(&(pSignal->request_pv));
	pSignal->cseq_number = t_rand() & 0xffff;
	pSignal->get_register_request_intermediate_state = dave_false;
	pSignal->register_request = NULL;
	pSignal->get_invite_request_intermediate_state = dave_false;
	pSignal->invite_request = NULL;
	pSignal->get_bye_request_intermediate_state = dave_false;
	pSignal->bye_request = NULL;

	pSignal->reg.reg_timer_counter = 0;
	dave_snprintf(kv_name, sizeof(kv_name), "sipcallidkv-%lx", pSignal);
	pSignal->call_id_kv = kv_malloc(kv_name, 0, NULL);
	dave_snprintf(kv_name, sizeof(kv_name), "sipcalldatakv-%lx", pSignal);
	pSignal->call_data_kv = kv_malloc(kv_name, 0, NULL);
}

static void
_sip_signal_signal_release(SIPSignal *pSignal)
{
	if(pSignal->signal_socket != INVALID_SOCKET_ID)
	{
		sip_client_release(pSignal->signal_socket);
		pSignal->signal_socket = INVALID_SOCKET_ID;
	}

	if(pSignal->register_request != NULL)
	{
		osip_message_free(pSignal->register_request);
		pSignal->register_request = NULL;
	}

	if(pSignal->invite_request != NULL)
	{
		osip_message_free(pSignal->invite_request);
		pSignal->invite_request = NULL;
	}

	if(pSignal->bye_request != NULL)
	{
		osip_message_free(pSignal->bye_request);
		pSignal->bye_request = NULL;
	}

	if(pSignal->call_id_kv != NULL)
	{
		kv_free(pSignal->call_id_kv, _sip_signal_call_recycle);
		pSignal->call_id_kv = NULL;
	}

	if(pSignal->call_data_kv != NULL)
	{
		kv_free(pSignal->call_data_kv, NULL);
		pSignal->call_data_kv = NULL;
	}
}

static SIPSignal *
_sip_signal_creat(
	s8 *server_ip, s8 *server_port, s8 *username, s8 *password,
	s8 *local_ip, s8 *local_port,
	s8 *rtp_ip, s8 *rtp_port)
{
	SIPSignalType signal_type;
	s32 signal_socket;
	SIPSignal *pSignal;

	if((server_ip != NULL) && (server_port != NULL))
	{
		signal_type = SIPSignalType_client;
		signal_socket = sip_client_creat(server_ip, server_port, local_port);
		if(signal_socket == INVALID_SOCKET_ID)
		{
			SIPLOG("server:%s:%s local:%s:%s creat failed!",
				server_ip, server_port, local_ip, local_port);
			return NULL;
		}
	}
	else if(local_port != NULL)
	{
		signal_type = SIPSignalType_server;
		signal_socket = sip_server_creat(local_port);
		if(signal_socket == INVALID_SOCKET_ID)
		{
			SIPLOG("server:%s:%s local:%s:%s creat failed!",
				server_ip, server_port, local_ip, local_port);
			return NULL;
		}		
	}
	else
	{
		SIPLOG("server_ip:%s server_port:%s local_port:%s",
			server_ip, server_port, local_port);
		return NULL;
	}

	pSignal = dave_ralloc(sizeof(SIPSignal));

	_sip_signal_signal_creat(
		pSignal,
		signal_type,
		signal_socket,
		server_ip, server_port, username, password,
		local_ip, local_port,
		rtp_ip, rtp_port);

	kv_add_ub_ptr(_sip_signal_socket_kv, pSignal->signal_socket, pSignal);

	sip_state_creat(pSignal);

	return pSignal;
}

static void
_sip_signal_release(SIPSignal *pSignal)
{
	sip_state_release(pSignal);

	kv_del_ub_ptr(_sip_signal_socket_kv, pSignal->signal_socket);

	_sip_signal_signal_release(pSignal);

	dave_free(pSignal);
}

static RetCode
_sip_signal_recycle(void *ramkv, s8 *key)
{
	SIPSignal *pSignal = (SIPSignal *)kv_del_key_ptr(_sip_signal_socket_kv, key);

	if(pSignal == NULL)
	{
		return RetCode_empty_data;
	}

	_sip_signal_release(pSignal);

	return RetCode_OK;
}

// =====================================================================

void
sip_signal_init(void)
{
	sip_global_lock_init();

	sip_state_init();

	_sip_signal_socket_kv = kv_malloc("sipsignalkv", 0, NULL);
}

void
sip_signal_exit(void)
{
	kv_free(_sip_signal_socket_kv, _sip_signal_recycle);
	_sip_signal_socket_kv = NULL;

	sip_state_exit();

	sip_global_lock_exit();
}

SIPSignal *
sip_signal_creat(
	s8 *server_ip, s8 *server_port, s8 *username, s8 *password,
	s8 *local_ip, s8 *local_port,
	s8 *rtp_ip, s8 *rtp_port)
{
	SIPSignal *pSignal;

	sip_global_lock();

	pSignal = _sip_signal_creat(
		server_ip, server_port, username, password,
		local_ip, local_port,
		rtp_ip, rtp_port);

	sip_state_creat(pSignal);

	sip_global_unlock();

	if(pSignal == NULL)
		return NULL;

	SIPLOG("server:%s:%s user:%s/%s local:%s:%s rtp:%s:%s",
		pSignal->server_ip, pSignal->server_port,
		pSignal->username, pSignal->password,
		pSignal->local_ip, pSignal->local_port,
		pSignal->rtp_ip, pSignal->rtp_port);

	if(pSignal->signal_type == SIPSignalType_client)
	{
		sip_reg(pSignal);
	}

	return pSignal;
}

void
sip_signal_release(SIPSignal *pSignal)
{
	SIPLOG("server:%s:%s user:%s/%s local:%s:%s rtp:%s:%s",
		pSignal->server_ip, pSignal->server_port,
		pSignal->username, pSignal->password,
		pSignal->local_ip, pSignal->local_port,
		pSignal->rtp_ip, pSignal->rtp_port);

	sip_global_lock();

	sip_state_release(pSignal);

	_sip_signal_release(pSignal);

	sip_global_unlock();
}

void
sip_signal_reg_reg(SIPSignal *pSignal, sip_recv_fun fun, void *param)
{
	pSignal->reg_recv_fun = fun;
	pSignal->reg_recv_param = param;
}

void
sip_signal_reg_inv(SIPSignal *pSignal, sip_recv_fun fun, void *param)
{
	pSignal->inv_recv_fun = fun;
	pSignal->inv_recv_param = param;
}

void
sip_signal_reg_bye(SIPSignal *pSignal, sip_recv_fun fun, void *param)
{
	pSignal->bye_recv_fun = fun;
	pSignal->bye_recv_param = param;
}

dave_bool
sip_signal_recv(SocketRead *pRead)
{
	SIPSignal *pSignal = kv_inq_ub_ptr(_sip_signal_socket_kv, pRead->socket);

	if(pSignal == NULL)
	{
		SIPLOG("invalid socket:%d", pRead->socket);
		return dave_false;
	}

	if((pRead->data->len + pSignal->recv_w_index) > SIP_RECV_BUFFER_MAX)
	{
		SIPLOG("buffer overflow! data len:%d w index:%d buffer max:%d",
			pRead->data->len, pSignal->recv_w_index, SIP_RECV_BUFFER_MAX);
		return dave_true;
	}

	pSignal->recv_w_index += dave_memcpy(&(pSignal->recv_buffer[pSignal->recv_w_index]), ms8(pRead->data), mlen(pRead->data));

	_sip_signal_recv(pSignal);

	if(pSignal->recv_r_index >= pSignal->recv_w_index)
	{
		pSignal->recv_r_index = pSignal->recv_e_index = pSignal->recv_w_index = 0;
	}

	return dave_true;
}

void
sip_signal_send(SIPSignal *pSignal, osip_message_t *sip)
{
	char *sip_string;
	size_t sip_length;
	MBUF *sip_mbuf;

	if((pSignal == NULL) || (sip == NULL))
	{
		SIPLOG("invalid pSignal:%lx sip:%lx", pSignal, sip);
		osip_message_free(sip);
		return;
	}

	osip_message_to_str(sip, &sip_string, &sip_length);
	if(sip_length == 0)
	{
		SIPLOG("invalid invite message!");
		osip_message_free(sip);
		return;
	}

	sip_mbuf = dave_mmalloc(sip_length);
	dave_memcpy(ms8(sip_mbuf), sip_string, sip_length);

	if(_sip_signal_is_re_requeset(pSignal, sip) == dave_false)
	{
		if(sip_automatic_send(pSignal, sip) == dave_false)
		{
			osip_message_free(sip);
		}
		else
		{
			SIPLOG("%s:%s->%s:%s call_id:%s %s@%s->%s@%s cseq:%s/%s",
				pSignal->server_ip, pSignal->server_port,
				pSignal->local_ip, pSignal->local_port,
				sip->call_id->number,
				sip->from->url->username, sip->from->url->host, sip->to->url->username, sip->to->url->host,
				sip->cseq->method, sip->cseq->number);
		}
	}

	_sip_signal_send(pSignal, sip_mbuf);
}


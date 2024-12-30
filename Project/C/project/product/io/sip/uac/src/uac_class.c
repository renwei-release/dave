/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_osip.h"
#include "uac_class.h"
#include "uac_client.h"
#include "uac_rtp.h"
#include "uac_reg.h"
#include "uac_state.h"
#include "uac_automatic.h"
#include "uac_global_lock.h"
#include "uac_log.h"

static void *_socket_class_kv = NULL;

static RetCode
_uac_class_recycle(void *ramkv, s8 *key)
{
	UACClass *pClass = (UACClass *)kv_del_key_ptr(_socket_class_kv, key);

	if(pClass == NULL)
	{
		return RetCode_empty_data;
	}

	dave_free(pClass);

	return RetCode_OK;
}

static osip_message_t *
_uac_class_message_parse(const char *buf, size_t length)
{
	osip_message_t *sip;
	int ret;

	if((length >= UAC_CLASS_RECV_BUFFER_MAX) || (length <= 4))
	{
		UACLOG("invalid length:%d", length);
		return NULL;
	}

	osip_message_init(&sip);

	ret = osip_message_parse(sip, buf, length);
	if(ret != OSIP_SUCCESS)
	{
		UACLOG("ret:%d data:%d/%s", ret, length, buf);
		return NULL;
	}

	return sip;
}

static int
_uac_class_message_distribution(UACClass *pClass, osip_message_t *sip)
{
	int status_code = 0;

	if((dave_strcmp(sip->cseq->method, "REGISTER") == dave_true)
		&& (pClass->signal.reg_recv_fun != NULL))
	{
		status_code = (int)pClass->signal.reg_recv_fun(pClass, sip);
	}
	else if((dave_strcmp(sip->cseq->method, "INVITE") == dave_true)
		&& (pClass->signal.inv_recv_fun != NULL))
	{
		status_code = (int)pClass->signal.inv_recv_fun(pClass, sip);
	}
	else if((dave_strcmp(sip->cseq->method, "BYE") == dave_true)
		&& (pClass->signal.bye_recv_fun != NULL))
	{
		status_code = (int)pClass->signal.bye_recv_fun(pClass, sip);
	}
	else
	{
		UACLOG("unprocess cseq:%s %s", sip->cseq->method, sip->cseq->number);
	}

	return status_code;
}

static ub
_uac_class_find_sip_package_end(UACClass *pClass)
{
	ub sip_package_end_index;
	ub sip_length, content_length;

	sip_package_end_index = osip_find_end_flag(&pClass->signal.recv_buffer[pClass->signal.recv_e_index], pClass->signal.recv_w_index - pClass->signal.recv_e_index);
	if(sip_package_end_index == 0)
	{
		/*
		 * At least 4 bytes of margin are required to prevent missed detection.
		 * \r\n\r\n
		 */
		if(pClass->signal.recv_w_index > 4)
			pClass->signal.recv_e_index = pClass->signal.recv_w_index - 4;
		else
			pClass->signal.recv_e_index = 0;
		return 0;
	}

	sip_package_end_index = pClass->signal.recv_e_index + sip_package_end_index;

	sip_length = sip_package_end_index - pClass->signal.recv_r_index;

	content_length = osip_content_length(&pClass->signal.recv_buffer[pClass->signal.recv_r_index], sip_length);

	if((pClass->signal.recv_w_index - pClass->signal.recv_r_index) >= (sip_length + content_length))
		return sip_package_end_index + content_length;

	return 0;
}

static dave_bool
_uac_class_is_re_requeset(UACClass *pClass, osip_message_t *sip)
{
	if(sip->sip_method == NULL)
		return dave_false;

	if(pClass->signal.register_request == sip)
		return dave_true;

	if(pClass->signal.invite_request == sip)
		return dave_true;

	if(pClass->signal.bye_request == sip)
		return dave_true;

	return dave_false;;
}

static void
_uac_class_recv(UACClass *pClass)
{
	ub sip_package_end_index;
	osip_message_t *sip;
	int status_code;

	sip_package_end_index = _uac_class_find_sip_package_end(pClass);
	if(sip_package_end_index == 0)
	{
		return;
	}

	sip = _uac_class_message_parse(&pClass->signal.recv_buffer[pClass->signal.recv_r_index], sip_package_end_index - pClass->signal.recv_r_index);
	pClass->signal.recv_r_index = pClass->signal.recv_e_index = sip_package_end_index;

	if(sip == NULL)
	{
		UACLOG("r:%d e:%d w:%d",
			pClass->signal.recv_r_index, pClass->signal.recv_e_index, pClass->signal.recv_w_index);
		return;
	}

	UACLOG("r:%d e:%d w:%d version:%s method:%s status_code:%d reason_phrase:%s",
		pClass->signal.recv_r_index, pClass->signal.recv_e_index, pClass->signal.recv_w_index,
		sip->sip_version, sip->sip_method, sip->status_code, sip->reason_phrase);

	status_code = _uac_class_message_distribution(pClass, sip);

	uac_automatic_recv(pClass, sip, status_code);

	osip_message_free(sip);
}

static void
_uac_class_send(UACClass *pClass, MBUF *pData)
{
	SocketWrite *pWrite = thread_msg(pWrite);

	pWrite->socket = pClass->signal.signal_socket;
	pWrite->IPInfo.protocol = IPProtocol_UDP;
	strip(pClass->signal.local_ip, 0, pWrite->IPInfo.src_ip, sizeof(pWrite->IPInfo.src_ip));
	pWrite->IPInfo.src_port = stringdigital(pClass->signal.local_port);
	strip(pClass->signal.server_ip, 0, pWrite->IPInfo.dst_ip, sizeof(pWrite->IPInfo.dst_ip));
	pWrite->IPInfo.dst_port = stringdigital(pClass->signal.server_port);
	pWrite->data_len = pData->tot_len;
	pWrite->data = pData;
	pWrite->close_flag = SOCKETINFO_SND;

	UACLOG("%s->%s data:%d",
		ipv4str(pWrite->IPInfo.src_ip, pWrite->IPInfo.src_port),
		ipv4str2(pWrite->IPInfo.dst_ip, pWrite->IPInfo.dst_port),
		pWrite->data_len);

	name_msg(SOCKET_THREAD_NAME, SOCKET_WRITE, pWrite);
}

static void
_uac_class_signal_creat(
	UACSignal *signal,
	s32 signal_socket,
	s8 *server_ip, s8 *server_port, s8 *username, s8 *password,
	s8 *local_ip, s8 *local_port,
	s8 *rtp_ip, s8 *rtp_port)
{
	signal->signal_socket = signal_socket;

	dave_strcpy(signal->server_ip, server_ip, sizeof(signal->server_ip));
	dave_strcpy(signal->server_port, server_port, sizeof(signal->server_port));
	dave_strcpy(signal->username, username, sizeof(signal->username));
	dave_strcpy(signal->password, password, sizeof(signal->password));	
	dave_strcpy(signal->local_ip, local_ip, sizeof(signal->local_ip));
	dave_strcpy(signal->local_port, local_port, sizeof(signal->local_port));
	dave_strcpy(signal->rtp_ip, rtp_ip, sizeof(signal->rtp_ip));
	dave_strcpy(signal->rtp_port, rtp_port, sizeof(signal->rtp_port));

	signal->recv_r_index = signal->recv_e_index = signal->recv_w_index = 0;

	signal->reg_recv_fun = signal->inv_recv_fun = signal->bye_recv_fun = NULL;

	t_lock_reset(&(signal->request_pv));
	signal->cseq_number = t_rand() & 0xffff;
	signal->get_register_request_intermediate_state = dave_false;
	signal->register_request = NULL;
	signal->get_invite_request_intermediate_state = dave_false;
	signal->invite_request = NULL;
	signal->get_bye_request_intermediate_state = dave_false;
	signal->bye_request = NULL;

	signal->reg_timer_counter = 0;
}

static void
_uac_class_signal_release(UACSignal *signal)
{
	if(signal->signal_socket != INVALID_SOCKET_ID)
	{
		uac_client_release(signal->signal_socket);
		signal->signal_socket = INVALID_SOCKET_ID;
	}

	if(signal->register_request != NULL)
	{
		osip_message_free(signal->register_request);
		signal->register_request = NULL;
	}

	if(signal->invite_request != NULL)
	{
		osip_message_free(signal->invite_request);
		signal->invite_request = NULL;
	}

	if(signal->bye_request != NULL)
	{
		osip_message_free(signal->bye_request);
		signal->bye_request = NULL;
	}
}

static UACClass *
_uac_class_creat(
	s8 *server_ip, s8 *server_port, s8 *username, s8 *password,
	s8 *local_ip, s8 *local_port,
	s8 *rtp_ip, s8 *rtp_port)
{
	s32 signal_socket;
	UACClass *pClass;

	signal_socket = uac_client_creat(server_ip, server_port, local_port);
	if(signal_socket == INVALID_SOCKET_ID)
	{
		UACLOG("server:%s:%s local:%s:%s creat failed!",
			server_ip, server_port, local_ip, local_port);
		return NULL;
	}

	pClass = dave_ralloc(sizeof(UACClass));

	_uac_class_signal_creat(
		&pClass->signal,
		signal_socket,
		server_ip, server_port, username, password,
		local_ip, local_port,
		rtp_ip, rtp_port);

	pClass->call.rtp = NULL;

	kv_add_ub_ptr(_socket_class_kv, pClass->signal.signal_socket, pClass);

	uac_state_creat(pClass);

	return pClass;
}

static void
_uac_class_release(UACClass *pClass)
{
	uac_state_release(pClass);

	kv_del_ub_ptr(_socket_class_kv, pClass->signal.signal_socket);

	_uac_class_signal_release(&pClass->signal);

	if(pClass->call.rtp != NULL)
	{
		uac_rtp_release(pClass->call.rtp);
		pClass->call.rtp = NULL;
	}

	dave_free(pClass);
}

// =====================================================================

void
uac_class_init(void)
{
	_socket_class_kv = kv_malloc("sckv", 0, NULL);
}

void
uac_class_exit(void)
{
	kv_free(_socket_class_kv, _uac_class_recycle);
	_socket_class_kv = NULL;
}

UACClass *
uac_class_creat(
	s8 *server_ip, s8 *server_port, s8 *username, s8 *password,
	s8 *local_ip, s8 *local_port,
	s8 *rtp_ip, s8 *rtp_port)
{
	UACClass *pClass;

	uac_global_lock();

	pClass = _uac_class_creat(
		server_ip, server_port, username, password,
		local_ip, local_port,
		rtp_ip, rtp_port);

	uac_global_unlock();

	if(pClass == NULL)
		return NULL;

	UACLOG("server:%s:%s user:%s/%s local:%s:%s rtp:%s:%s",
		pClass->signal.server_ip, pClass->signal.server_port,
		pClass->signal.username, pClass->signal.password,
		pClass->signal.local_ip, pClass->signal.local_port,
		pClass->signal.rtp_ip, pClass->signal.rtp_port);

	return pClass;
}

void
uac_class_release(UACClass *pClass)
{
	UACLOG("server:%s:%s user:%s/%s local:%s:%s rtp:%s:%s",
		pClass->signal.server_ip, pClass->signal.server_port,
		pClass->signal.username, pClass->signal.password,
		pClass->signal.local_ip, pClass->signal.local_port,
		pClass->signal.rtp_ip, pClass->signal.rtp_port);

	uac_global_lock();

	_uac_class_release(pClass);

	uac_global_unlock();
}

void
uac_class_reg_reg(void *pClass, uac_recv_fun fun)
{
	((UACClass *)pClass)->signal.reg_recv_fun = fun;
}

void
uac_class_reg_inv(void *pClass, uac_recv_fun fun)
{
	((UACClass *)pClass)->signal.inv_recv_fun = fun;
}

void
uac_class_reg_bye(void *pClass, uac_recv_fun fun)
{
	((UACClass *)pClass)->signal.bye_recv_fun = fun;
}

void
uac_class_recv(SocketRead *pRead)
{
	UACClass *pClass = kv_inq_ub_ptr(_socket_class_kv, pRead->socket);

	if(pClass == NULL)
	{
		UACLOG("invalid socket:%d", pRead->socket);
		return;
	}

	if((pRead->data->len + pClass->signal.recv_w_index) > UAC_CLASS_RECV_BUFFER_MAX)
	{
		UACLOG("buffer overflow! data len:%d w index:%d buffer max:%d",
			pRead->data->len, pClass->signal.recv_w_index, UAC_CLASS_RECV_BUFFER_MAX);
		return;
	}

	pClass->signal.recv_w_index += dave_memcpy(&(pClass->signal.recv_buffer[pClass->signal.recv_w_index]), ms8(pRead->data), mlen(pRead->data));

	_uac_class_recv(pClass);

	if(pClass->signal.recv_r_index >= pClass->signal.recv_w_index)
	{
		pClass->signal.recv_r_index = pClass->signal.recv_e_index = pClass->signal.recv_w_index = 0;
	}
}

void
uac_class_send(UACClass *pClass, osip_message_t *sip)
{
	char *sip_string;
	size_t sip_length;
	MBUF *sip_mbuf;

	if((pClass == NULL) || (sip == NULL))
	{
		UACLOG("invalid pClass:%lx sip:%lx", pClass, sip);
		osip_message_free(sip);
		return;
	}

	osip_message_to_str(sip, &sip_string, &sip_length);
	if(sip_length == 0)
	{
		UACLOG("invalid invite message!");
		osip_message_free(sip);
		return;
	}

	sip_mbuf = dave_mmalloc(sip_length);
	dave_memcpy(ms8(sip_mbuf), sip_string, sip_length);

	if(_uac_class_is_re_requeset(pClass, sip) == dave_false)
	{
		if(uac_automatic_send(pClass, sip) == dave_false)
		{
			osip_message_free(sip);
		}
		else
		{
			UACLOG("%s:%s->%s:%s call_id:%s %s@%s->%s@%s cseq:%s/%s",
				pClass->signal.server_ip, pClass->signal.server_port,
				pClass->signal.local_ip, pClass->signal.local_port,
				sip->call_id->number,
				sip->from->url->username, sip->from->url->host, sip->to->url->username, sip->to->url->host,
				sip->cseq->method, sip->cseq->number);
		}
	}

	_uac_class_send(pClass, sip_mbuf);
}


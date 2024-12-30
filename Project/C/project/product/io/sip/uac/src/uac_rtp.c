/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_uac.h"
#include "dave_osip.h"
#include "uac_class.h"
#include "uac_server.h"
#include "uac_client.h"
#include "uac_rtp.h"
#include "uac_rtp_msg.h"
#include "uac_log.h"

static void *_socket_rtp_kv = NULL;
static void *_call_id_rtp_kv = NULL;

static void
_uac_rtp_parse_rtp_header(const u8 *packet, RTPHeadr *header) 
{
    header->version = (packet[0] >> 6) & 0x03;
    header->padding = (packet[0] >> 5) & 0x01;
    header->extension = (packet[0] >> 4) & 0x01;
    header->csrc_count = packet[0] & 0x0F;
    header->marker = (packet[1] >> 7) & 0x01;
    header->payload_type = packet[1] & 0x7F;
    header->sequence_number = (packet[2] << 8) | packet[3];
    header->timestamp = (packet[4] << 24) | (packet[5] << 16) | (packet[6] << 8) | packet[7];
    header->ssrc = (packet[8] << 24) | (packet[9] << 16) | (packet[10] << 8) | packet[11];

    for (int i = 0; i < header->csrc_count; i++)
	{
        header->csrc[i] = (packet[12 + i * 4] << 24) | (packet[13 + i * 4] << 16) | (packet[14 + i * 4] << 8) | packet[15 + i * 4];
    }
}

static MBUF *
_uac_rtp_build_rtp_header(RTPHeadr *header, MBUF *payload_data)
{
	ub head_len = RTP_HEADER_LEN + (header->csrc_count * 4);
	MBUF *rtp_data;
	u8 *packet;

	rtp_data = dave_mmalloc(head_len + payload_data->len);

	packet = (u8 *)(rtp_data->payload);

	packet[0] = (header->version << 6) | (header->padding << 5) | (header->extension << 4) | header->csrc_count;
	packet[1] = (header->marker << 7) | header->payload_type;
	packet[2] = header->sequence_number >> 8;
	packet[3] = header->sequence_number & 0xFF;
	packet[4] = header->timestamp >> 24;
	packet[5] = (header->timestamp >> 16) & 0xFF;
	packet[6] = (header->timestamp >> 8) & 0xFF;
	packet[7] = header->timestamp & 0xFF;
	packet[8] = header->ssrc >> 24;
	packet[9] = (header->ssrc >> 16) & 0xFF;
	packet[10] = (header->ssrc >> 8) & 0xFF;
	packet[11] = header->ssrc & 0xFF;

	for (int i = 0; i < header->csrc_count; i++)
	{
		packet[12 + i * 4] = header->csrc[i] >> 24;
		packet[13 + i * 4] = (header->csrc[i] >> 16) & 0xFF;
		packet[14 + i * 4] = (header->csrc[i] >> 8) & 0xFF;
		packet[15 + i * 4] = header->csrc[i] & 0xFF;
	}

	dave_memcpy(&packet[head_len], payload_data->payload, payload_data->len);

	return rtp_data;
}

static RetCode
_uac_rtp_recycle(void *ramkv, s8 *key)
{
	UACRTP *pRTP = (UACRTP *)kv_del_key_ptr(_socket_rtp_kv, key);

	if(pRTP == NULL)
	{
		return RetCode_empty_data;
	}

	dave_free(pRTP);

	return RetCode_OK;
}

static void
_uac_rtp_send(UACRTP *pRTP, MBUF *pData)
{
	if(pRTP->send_rtp_socket == INVALID_SOCKET_ID)
	{
		dave_mfree(pData);
		return;
	}

	SocketWrite *pWrite = thread_msg(pWrite);

	pWrite->socket = pRTP->send_rtp_socket;
	pWrite->IPInfo.protocol = IPProtocol_UDP;
	dave_memcpy(pWrite->IPInfo.src_ip, pRTP->u8_local_rtp_ip, 4);
	pWrite->IPInfo.src_port = pRTP->u16_local_rtp_port;
	dave_memcpy(pWrite->IPInfo.dst_ip, pRTP->u8_remote_rtp_ip, 4);
	pWrite->IPInfo.dst_port = pRTP->u16_remote_rtp_port;
	pWrite->data_len = pData->tot_len;
	pWrite->data = pData;
	pWrite->close_flag = SOCKETINFO_SND;

	UACDEBUG("%s->%s data:%d",
		ipv4str(pWrite->IPInfo.src_ip, pWrite->IPInfo.src_port),
		ipv4str2(pWrite->IPInfo.dst_ip, pWrite->IPInfo.dst_port),
		pWrite->data_len);

	name_msg(SOCKET_THREAD_NAME, SOCKET_WRITE, pWrite);
}

static void
_uac_rtp_recv(UACRTP *pRTP, s8 *rtp_data_ptr, ub rtp_data_len)
{
	RTPHeadr rtp_header;
	RTPDATA rsp_rtp_data;

	if(rtp_data_len < RTP_HEADER_LEN)
	{
		UACLOG("rtp data len:%d < 12", rtp_data_len);
		return;
	}

	_uac_rtp_parse_rtp_header((const u8 *)rtp_data_ptr, &rtp_header);

    size_t rtp_header_len = RTP_HEADER_LEN + rtp_header.csrc_count * 4;
    size_t payload_len = rtp_data_len - rtp_header_len;
    const uint8_t *payload_ptr = (const uint8_t *)(&rtp_data_ptr[rtp_header_len]);

	rsp_rtp_data = uac_rtp_msg_data(
		pRTP,
		rtp_header.payload_type,
		rtp_header.sequence_number, rtp_header.timestamp, rtp_header.ssrc,
		(s8 *)payload_ptr, payload_len);

	uac_rtp_send(
		pRTP,
		rsp_rtp_data.payload_type,
		rsp_rtp_data.sequence_number, rsp_rtp_data.timestamp, rsp_rtp_data.ssrc,
		rsp_rtp_data.payload_data);
}

static s32
_uac_rtp_bind(s8 *rtp_port_ptr, ub rtp_port_len)
{
	s32 rtp_socket = INVALID_SOCKET_ID;
	ub ub_rtp_port;
	ub counter;

	counter = 0;

	while((rtp_socket == INVALID_SOCKET_ID) && ((++ counter) < 32))
	{
		rtp_socket = uac_server_creat(rtp_port_ptr);
		if(rtp_socket == INVALID_SOCKET_ID)
		{
			UACLOG("bind rtp port:%s failed!", rtp_port_ptr);

			ub_rtp_port = stringdigital(rtp_port_ptr) + 1;
			dave_snprintf(rtp_port_ptr, rtp_port_len, "%d", ub_rtp_port);
		}
	}

	return rtp_socket;
}

static void
_uac_rtp_clean_socket(UACRTP *pRTP)
{
	s32 socket;

	if(pRTP->recv_rtp_socket != INVALID_SOCKET_ID)
	{
		socket = pRTP->recv_rtp_socket;
		pRTP->recv_rtp_socket = INVALID_SOCKET_ID;
	
		kv_del_ub_ptr(_socket_rtp_kv, socket);
		uac_server_release(socket);
	}
	if(pRTP->send_rtp_socket != INVALID_SOCKET_ID)
	{
		socket = pRTP->send_rtp_socket;
		pRTP->send_rtp_socket = INVALID_SOCKET_ID;

		kv_del_ub_ptr(_socket_rtp_kv,socket);
		uac_client_release(socket);
	}
}

static void
_uac_rtp_delay_release(TIMERID timer_id, ub thread_index, void *param_ptr)
{
	UACRTP *pRTP = (UACRTP *)param_ptr;

	dave_free(pRTP);

	base_timer_die(timer_id);
}

// =====================================================================

void
uac_rtp_init(void)
{
	_socket_rtp_kv = kv_malloc("srkv", 0, NULL);
	_call_id_rtp_kv = kv_malloc("crkv", 0, NULL);
}

void
uac_rtp_exit(void)
{
	kv_free(_call_id_rtp_kv, NULL);
	kv_free(_socket_rtp_kv, _uac_rtp_recycle);
	_socket_rtp_kv = NULL;
}

UACRTP *
uac_rtp_creat(s8 *local_rtp_ip, s8 *local_rtp_port)
{
	s8 rtp_port[16];
	s32 recv_rtp_socket;
	UACRTP *pRTP;

	dave_strcpy(rtp_port, local_rtp_port, sizeof(rtp_port));

	recv_rtp_socket = _uac_rtp_bind(rtp_port, sizeof(rtp_port));
	if(recv_rtp_socket == INVALID_SOCKET_ID)
	{
		UACABNOR("rtp send creat failed! local:%s:%s",
			local_rtp_ip, local_rtp_port);
		return NULL;
	}

	pRTP = dave_ralloc(sizeof(UACRTP));

	pRTP->recv_rtp_socket = recv_rtp_socket;
	pRTP->send_rtp_socket = INVALID_SOCKET_ID;

	dave_strcpy(pRTP->local_rtp_ip, local_rtp_ip, sizeof(pRTP->local_rtp_ip));
	dave_strcpy(pRTP->local_rtp_port, rtp_port, sizeof(pRTP->local_rtp_port));

	t_lock_reset(&(pRTP->rtp_data_pv));
	pRTP->payload_type = 8;
	pRTP->sequence_number = 0;
	pRTP->timestamp = 0;
	pRTP->ssrc = (u32)t_rand();
	pRTP->rtp_data_r_index = pRTP->rtp_data_w_index = 0;
	pRTP->current_buffer_ssrc = 0;
	pRTP->rtp_data_buffer[0] = '\0';

	kv_add_ub_ptr(_socket_rtp_kv, pRTP->recv_rtp_socket, pRTP);

	UACLOG("rtp creat success! socket:%d/%d local:%s:%s",
		pRTP->recv_rtp_socket, pRTP->send_rtp_socket,
		pRTP->local_rtp_ip, pRTP->local_rtp_port);

	return pRTP;
}

void
uac_rtp_call_id_build(UACRTP *pRTP, s8 *call_id, s8 *call_from, s8 *call_to)
{
	dave_strcpy(pRTP->call_id, call_id, sizeof(pRTP->call_id));
	dave_strcpy(pRTP->call_from, call_from, sizeof(pRTP->call_from));
	dave_strcpy(pRTP->call_to, call_to, sizeof(pRTP->call_to));

	uac_rtp_msg_start(pRTP->call_id, pRTP->call_from, pRTP->call_to);

	kv_add_key_ptr(_call_id_rtp_kv, call_id, pRTP);
}

void
uac_rtp_send_build(UACRTP *pRTP, s8 *remote_rtp_ip, s8 *remote_rtp_port)
{
	_uac_rtp_clean_socket(pRTP);

	strip(pRTP->local_rtp_ip, 0, pRTP->u8_local_rtp_ip, sizeof(pRTP->u8_local_rtp_ip));
	pRTP->u16_local_rtp_port = stringdigital(pRTP->local_rtp_port);
	strip(pRTP->remote_rtp_ip, 0, pRTP->u8_remote_rtp_ip, sizeof(pRTP->u8_remote_rtp_ip));
	pRTP->u16_remote_rtp_port = stringdigital(pRTP->remote_rtp_port);

	pRTP->send_rtp_socket = uac_client_creat(remote_rtp_ip, remote_rtp_port, pRTP->local_rtp_port);
	if(pRTP->send_rtp_socket == INVALID_SOCKET_ID)
	{
		UACABNOR("remote:%s:%s", remote_rtp_ip, remote_rtp_port);
		return;
	}

	dave_strcpy(pRTP->remote_rtp_ip, remote_rtp_ip, sizeof(pRTP->remote_rtp_ip));
	dave_strcpy(pRTP->remote_rtp_port, remote_rtp_port, sizeof(pRTP->remote_rtp_port));

	kv_add_ub_ptr(_socket_rtp_kv, pRTP->send_rtp_socket, pRTP);

	UACLOG("rtp send build success! socket:%d/%d remote:%s:%s local:%s:%s",
		pRTP->recv_rtp_socket, pRTP->send_rtp_socket,
		pRTP->remote_rtp_ip, pRTP->remote_rtp_port,
		pRTP->local_rtp_ip, pRTP->local_rtp_port);
}

void
uac_rtp_release(UACRTP *pRTP)
{
	s8 release_timer_name[64];

	_uac_rtp_clean_socket(pRTP);

	uac_rtp_msg_stop(pRTP->call_id, pRTP->call_from, pRTP->call_to);

	kv_del_key_ptr(_call_id_rtp_kv, pRTP->call_id);

	dave_snprintf(release_timer_name, sizeof(release_timer_name), "rtp%lx", pRTP);

	base_timer_param_creat(release_timer_name, _uac_rtp_delay_release, pRTP, sizeof(void *), 3000);
}

dave_bool
uac_rtp_recv(SocketRead *pRead)
{
	UACRTP *pRTP = kv_inq_ub_ptr(_socket_rtp_kv, pRead->socket);

	if(pRTP == NULL)
	{
		return dave_false;
	}

	_uac_rtp_recv(pRTP, ms8(pRead->data), mlen(pRead->data));

	return dave_true;
}

void
uac_rtp_send(
	UACRTP *pRTP,
	u8 payload_type,
	u16 sequence_number, u32 timestamp, u32 ssrc,
	MBUF *payload_data)
{
	RTPHeadr rtp_header;
	MBUF *rtp_data;

	if((pRTP == NULL) || (payload_data == NULL))
	{
		UACLOG("invalid pRTP:%lx payload:%lx",
			pRTP, payload_data);
		return;
	}

	if(pRTP->send_rtp_socket != INVALID_SOCKET_ID)
	{
		rtp_header.version = 2;
		rtp_header.padding = 0;
		rtp_header.extension = 0;
		rtp_header.csrc_count = 0;
		rtp_header.marker = 0;
		rtp_header.payload_type = payload_type;
		rtp_header.sequence_number = sequence_number;
		rtp_header.timestamp = timestamp;
		rtp_header.ssrc = ssrc;

		rtp_data = _uac_rtp_build_rtp_header(&rtp_header, payload_data);

		_uac_rtp_send(pRTP, rtp_data);
	}
}

UACRTP *
uac_rtp_call_id_to_rtp(s8 *call_id)
{
	return (UACRTP *)kv_inq_key_ptr(_call_id_rtp_kv, call_id);
}


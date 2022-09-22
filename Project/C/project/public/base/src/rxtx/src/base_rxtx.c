/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_tools.h"
#include "dave_os.h"
#include "base_rxtx.h"
#include "base_tools.h"
#include "rxtx_confirm_transfer.h"
#include "rxtx_secure.h"
#include "rxtx_system.h"
#include "rxtx_param.h"
#include "rxtx_tools.h"
#include "rxtx_log.h"

#define SECURE_DATA_MAX_LENGTH 32
#define RECV_COUNTER_MAX (256)
#define MAYBE_HAS_DATA_COUNTER_MAX (1)
#define RXTX_BUFFER_CFG_NAME "RxTxBufferCfgLength"

typedef struct {
	RXTX *pRxTx;
	u8 *permutation_ptr;
	ub permutation_len;
	ub event_serial;
} RXTXEvent;

static ThreadId _socket_thread = INVALID_THREAD_ID;
static TLock _opt_pv;
static ub _rxtx_buffer_cfg_length = RX_TX_BUF_SETUP;
static RXTX _rx_tx[RXTX_MAX];

static inline void
_base_rxtx_reset(RXTX *pRxTx)
{
	pRxTx->type = TYPE_SOCK_max;
	pRxTx->socket = INVALID_SOCKET_ID;
	pRxTx->port = 0;
	dave_memset(&(pRxTx->IPInfo), 0x00, sizeof(IPBaseInfo));
	pRxTx->rx_buffer_ptr = NULL;
	pRxTx->rx_buffer_len = 0;
	pRxTx->receive_fun = NULL;
	pRxTx->param = NULL;
	pRxTx->owner_thread = INVALID_THREAD_ID;
	pRxTx->owner_file_name = NULL;
	pRxTx->owner_file_line = 0;
	pRxTx->enable_data_crc = dave_true;
}

static inline RXTX *
_base_rxtx_find_free(s32 socket)
{
	ub rxtx_index, count;

	if(socket <= INVALID_SOCKET_ID)
	{
		RTABNOR("invalid socket:%d", socket);
		return NULL;
	}

	rxtx_index = socket % RXTX_MAX;

	for(count=0; count<RXTX_MAX; count++)
	{
		if(rxtx_index >= RXTX_MAX)
		{
			rxtx_index = 0;
		}

		if(_rx_tx[rxtx_index].socket == INVALID_SOCKET_ID)
		{
			_rx_tx[rxtx_index].socket = socket;
			_rx_tx[rxtx_index].rxtx_index = rxtx_index;
			break;
		}

		rxtx_index ++;
	}

	if(count >= RXTX_MAX)
	{
		return NULL;
	}
	else
	{
		return &_rx_tx[rxtx_index];
	}
}

static inline RXTX *
_base_rxtx_find_busy(s32 socket)
{
	ub rxtx_index, count;

	rxtx_index = socket % RXTX_MAX;

	for(count=0; count<RXTX_MAX; count++)
	{
		if(rxtx_index >= RXTX_MAX)
		{
			rxtx_index = 0;
		}

		if(_rx_tx[rxtx_index].socket == socket)
		{
			break;
		}

		rxtx_index ++;
	}

	if(count >= RXTX_MAX)
	{
		return NULL;
	}
	else
	{
		return &_rx_tx[rxtx_index];
	}
}

static inline ub
_base_rxtx_set_magic_data(u8 *magic_data)
{
	magic_data[0] = 0x12;
	magic_data[1] = 0x34;
	magic_data[2] = 0x56;

	return 3;
}

static inline dave_bool
_base_rxtx_check_magic_data(u8 *magic_data)
{
	if((magic_data[0] != 0x12) || (magic_data[1] != 0x34) || (magic_data[2] != 0x56))
	{
		RTTRACE("invalid magic:%x,%x,%x", magic_data[0], magic_data[1], magic_data[2]);
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

static inline void
_base_rxtx_maybe_has_data(RXTX *pRxTx, SocketRawEvent *pUpEvent)
{	
	SocketRawEvent *pEvent = thread_msg(pEvent);

	pEvent->socket = pUpEvent->socket;
	pEvent->os_socket = pUpEvent->os_socket;
	pEvent->event = SOC_EVENT_REV;
	T_CopyNetInfo(&(pEvent->NetInfo), &(pUpEvent->NetInfo));
	pEvent->data = NULL;
	pEvent->ptr = pUpEvent->ptr;

	id_nmsg(pRxTx->owner_thread, SOCKET_RAW_EVENT, pEvent, 128);
}

static inline FRAMETYPE
_base_rxtx_to_type(u8 data_type)
{
	FRAMETYPE type;

	switch(data_type)
	{
		case RXTX_STACK_REL_VERTYPE:
				type = REL_FRAME;
			break;
		case RXTX_STACK_REL_SECRET_VERTYPE:
				type = ENCRYPT_REL_FRAME;
			break;
		case RXTX_STACK_CT_DATA_SECRET_VERTYPE:
				type = CT_ENCRYPT_DATA_FRAME;
			break;
		case RXTX_STACK_CT_ACK_VERTYPE:
				type = CT_ACK_FRAME;
			break;
		case RXTX_STACK_CT_SYNC_VERTYPE:
				type = CT_SYNC_FRAME;
			break;
		case RXTX_STACK_CT_DATA_VERTYPE:
				type = CT_DATA_FRAME;
			break;
		default:
				type = BAD_FRAME;
			break;
	}

	return type;
}

static inline FRAMETYPE
_base_rxtx_msgtype_(BinStackMsgHead *pMessage, u8 *data, ub len)
{
	FRAMETYPE type;
	u8 *magic_data_start;

	if(len < STACK_HEAD_LENver2)
	{
		type = BAD_LENGTH;
	}
	else
	{
		pMessage->ver_type = data[0];

		dave_byte_16(pMessage->order_id, data[1], data[2]);

		magic_data_start = &data[7];

		if((pMessage->order_id <= ORDER_CODE_BEGIN)
			|| (pMessage->order_id >= ORDER_CODE_END)
			|| (_base_rxtx_check_magic_data(magic_data_start) == dave_false))
		{
			RTTRACE("ver_type:%x order_id:%x", pMessage->ver_type, pMessage->order_id);
			type = BAD_FRAME;
		}
		else
		{
			dave_byte_8_32(pMessage->frame_len, data[3], data[4], data[5], data[6]);
			pMessage->frame = &data[STACK_HEAD_LENver2];
			if(len >= (ub)(STACK_HEAD_LENver2 + pMessage->frame_len))
			{
				RTDEBUG("frame_len:%d len:%d", pMessage->frame_len, len);
				type = _base_rxtx_to_type(data[0]);
			}
			else
			{
				type = BAD_LENGTH;
			}
		}
	}

	return type;
}

static inline FRAMETYPE
_base_rxtx_msgtype(BinStackMsgHead *message, u8 *data, ub len)
{
	FRAMETYPE type;

	message->ver_type = 0;
	message->order_id = 0;
	message->frame_len = 0;
	message->frame = NULL;

	switch(data[0])
	{
		case RXTX_STACK_REL_VERTYPE:
		case RXTX_STACK_REL_SECRET_VERTYPE:
		case RXTX_STACK_CT_DATA_SECRET_VERTYPE:
		case RXTX_STACK_CT_ACK_VERTYPE:
		case RXTX_STACK_CT_SYNC_VERTYPE:
		case RXTX_STACK_CT_DATA_VERTYPE:
				type = _base_rxtx_msgtype_(message, data, len);
			break;
		default:
				RTDEBUG("bad frame:%x", data[0]);
				type = BAD_FRAME;
			break;	
	}

	return type;
}

static inline void
_base_rxtx_has_bad_frame(RXTX *pRxTx, ub bad_frame_index, u8 *data_ptr, ub process_index, ub data_len)
{
	s8 *bad_data_file;
	ub bad_data_length, bad_data_index, data_index;
	s8 file_name[256];
	DateStruct file_time;

	RTLOG("thread:%s socket:%d has bad frame start index:%d, data len:%d/%d (%s:%d)",
		thread_name(pRxTx->owner_thread), pRxTx->socket,
		bad_frame_index, process_index, data_len,
		pRxTx->owner_file_name, pRxTx->owner_file_line);

	bad_data_length = data_len * 16 + 16;

	bad_data_file = dave_malloc(bad_data_length);

	for(data_index=0,bad_data_index=0; data_index<data_len; data_index++)
	{
		bad_data_index += dave_snprintf(&bad_data_file[bad_data_index], bad_data_length-bad_data_index,
			"0x%02x, ", data_ptr[data_index]);
	}

	t_time_get_date(&file_time);

	dave_snprintf(file_name, sizeof(file_name), "BAD_FRAME-%s-%d-%d-%d-%d-%d-%d-%d",
		thread_name(pRxTx->owner_thread),
		file_time.year, file_time.month, file_time.day,
		file_time.hour, file_time.minute, file_time.second,
		data_len);

	dave_os_file_write(CREAT_WRITE_FLAG, file_name, 0, bad_data_index, (u8 *)bad_data_file);

	dave_free(bad_data_file);
}

static inline ub
_base_rxtx_build_head(u8 *head_data, u8 ver_type, ORDER_CODE order_id, ub data_length)
{
	ub head_index = 0;

	ver_type &= 0x0f;
	ver_type |= (RXTX_STACK_VERSION << 4);

	head_data[head_index ++] = ver_type;
	dave_byte_8(head_data[head_index ++], head_data[head_index ++], order_id);
	dave_byte_32_8(head_data[head_index ++], head_data[head_index ++], head_data[head_index ++], head_data[head_index ++], (u32)(data_length));
	head_index += _base_rxtx_set_magic_data(&head_data[head_index]);
	if(head_index != STACK_HEAD_LENver2)
	{
		RTABNOR("invalid head_index:%d/%d", head_index, STACK_HEAD_LENver2);
	}

	return head_index;
}

static inline void
_base_rxtx_buffer_malloc(RXTX *pRxTx)
{
	if(pRxTx->rx_buffer_ptr == NULL)
	{
		pRxTx->rx_buffer_ptr = dave_malloc(_rxtx_buffer_cfg_length);
		pRxTx->rx_buffer_len = 0;
	}
}

static inline void
_base_rxtx_buffer_free(RXTX *pRxTx)
{
	if((pRxTx->rx_buffer_ptr != NULL) && (pRxTx->rx_buffer_len == 0))
	{
		dave_free(pRxTx->rx_buffer_ptr);
		pRxTx->rx_buffer_ptr = NULL;
		pRxTx->rx_buffer_len = 0;
	}
}

static inline void
_base_rxtx_buffer_tidy(RXTX *pRxTx, ub process_len)
{
	if(process_len > pRxTx->rx_buffer_len)
	{
		RTABNOR("process_len:%d > rx_buffer_len:%d why?",
			process_len, pRxTx->rx_buffer_len);

		pRxTx->rx_buffer_len = 0;
	}
	else if(process_len == pRxTx->rx_buffer_len)
	{
		pRxTx->rx_buffer_len = 0;
	}
	else if(process_len > 0)
	{
		pRxTx->rx_buffer_len = pRxTx->rx_buffer_len - process_len;

		dave_memmove(pRxTx->rx_buffer_ptr, &(pRxTx->rx_buffer_ptr[process_len]), pRxTx->rx_buffer_len);

		if(pRxTx->rx_buffer_len < _rxtx_buffer_cfg_length)
		{
			pRxTx->rx_buffer_ptr[pRxTx->rx_buffer_len] = '\0';
		}
	}
}

static inline void
_base_rxtx_buffer_permutation(u8 **permutation_ptr, ub *permutation_len, RXTX *pRxTx, ub process_len)
{
	u8 *last_data_ptr;
	ub last_data_len;

	if(process_len > pRxTx->rx_buffer_len)
	{
		RTABNOR("process_len:%d > rx_buffer_len:%d why?",
			process_len, pRxTx->rx_buffer_len);
		process_len = pRxTx->rx_buffer_len;
	}

	last_data_ptr = &pRxTx->rx_buffer_ptr[process_len];
	last_data_len = pRxTx->rx_buffer_len - process_len;

	*permutation_ptr = pRxTx->rx_buffer_ptr;
	*permutation_len = process_len;

	if(last_data_len > 0)
	{
		pRxTx->rx_buffer_ptr = dave_malloc(_rxtx_buffer_cfg_length);
		pRxTx->rx_buffer_len = last_data_len;
		dave_memcpy(pRxTx->rx_buffer_ptr, last_data_ptr, last_data_len);

		if(pRxTx->rx_buffer_len < _rxtx_buffer_cfg_length)
		{
			pRxTx->rx_buffer_ptr[pRxTx->rx_buffer_len] = '\0';
		}
	}
	else
	{
		pRxTx->rx_buffer_ptr = dave_malloc(_rxtx_buffer_cfg_length);
		pRxTx->rx_buffer_len = 0;
	}
}

static inline void
_base_rxtx_buffer_clean(u8 *permutation_ptr)
{
	dave_free(permutation_ptr);
}

static inline dave_bool
_base_rxtx_output(u8 ver_type, RXTX *pRxTx, u8 *dst_ip, u16 dst_port, ORDER_CODE order_id, MBUF *data)
{
	SocketWrite *pWrite;
	ub data_length = (data == NULL) ? 0 : data->tot_len;

	if((pRxTx->socket == INVALID_SOCKET_ID) || (pRxTx->socket < 0))
	{
		RTABNOR("invalid socket:%d on thread:%s port:%d",
				pRxTx->socket,
				thread_name(pRxTx->owner_thread),
				pRxTx->port);
		return dave_false;
	}

	pWrite = thread_msg(pWrite);

	pWrite->socket = pRxTx->socket;

	if(pRxTx->type == TYPE_SOCK_STREAM)
	{
		pWrite->IPInfo.protocol = IPProtocol_TCP;
	}
	else
	{
		pWrite->IPInfo.protocol = IPProtocol_UDP;
	}
	pWrite->IPInfo.ver = IPVER_IPV4;
	pWrite->IPInfo.src_port = 0;
	if(dst_ip != NULL)
	{
		dave_memcpy(pWrite->IPInfo.dst_ip, dst_ip, 4);
	}
	pWrite->IPInfo.dst_port = dst_port;
	pWrite->IPInfo.netcard_name[0] = '\0';
	pWrite->IPInfo.fixed_port_flag = NotFixedPort;

	pWrite->data = dave_mmalloc(STACK_HEAD_LENver2);

	pWrite->data->tot_len = pWrite->data->len =
		_base_rxtx_build_head((u8 *)(pWrite->data->payload), ver_type, order_id, data_length);

	if(data != NULL)
	{
		pWrite->data = dave_mchain(pWrite->data, data);
	}

	pWrite->data_len = pWrite->data->tot_len;

	pWrite->close_flag = SOCKETINFO_SND;

	return snd_from_msg(pRxTx->owner_thread, _socket_thread, SOCKET_WRITE, sizeof(SocketWrite), pWrite);
}

static inline void
_base_rxtx_output_ack(RXTX *pRxTx, ORDER_CODE order_id)
{
	RTDEBUG("order_id:%x port:%d", order_id, pRxTx->port);

	_base_rxtx_output(RXTX_STACK_CT_ACK_VERTYPE, pRxTx, pRxTx->IPInfo.src_ip, pRxTx->IPInfo.src_port, order_id, NULL);
}

static inline void
_base_rxtx_output_sync(RXTX *pRxTx, ORDER_CODE order_id)
{
	_base_rxtx_output(RXTX_STACK_CT_SYNC_VERTYPE, pRxTx, pRxTx->IPInfo.src_ip, pRxTx->IPInfo.src_port, order_id, NULL);
}

static inline dave_bool
_base_rxtx_output_data_encode(RXTX *pRxTx, u8 *dst_ip, u16 dst_port, u8 type, ORDER_CODE order_id, MBUF *data)
{
	MBUF *encode_package;
	dave_bool ret;

	encode_package = rxtx_simple_encode_request(data);

	ret = _base_rxtx_output(type, pRxTx, dst_ip, dst_port, order_id, encode_package);
	if(ret == dave_false)
	{
		RTABNOR("invalid socket:%d on thread:%s port:%d",
				pRxTx->socket,
				thread_name(pRxTx->owner_thread),
				pRxTx->port);
		rxtx_simple_encode_release(encode_package);
	}

	dave_mfree(data);

	return dave_true;
}

static inline dave_bool
_base_rxtx_output_data(RXTX *pRxTx, u8 *dst_ip, u16 dst_port, u8 type, ORDER_CODE order_id, MBUF *data)
{
	MBUF *crc;
	dave_bool ret;

	crc = dave_mmalloc(16);

	if(pRxTx->enable_data_crc == dave_true)
		crc->tot_len = crc->len = rxtx_build_crc_on_mbuf(data, dave_mptr(crc), crc->len);
	else
		crc->tot_len = crc->len = rxtx_build_crc_on_mbuf(NULL, dave_mptr(crc), crc->len);

	data = dave_mchain(data, crc);

	RTDEBUG("len:%d/%d/%d", data->tot_len, data->len, crc->len);

	ret = _base_rxtx_output(type, pRxTx, dst_ip, dst_port, order_id, data);
	if(ret == dave_false)
	{
		RTLTRACE(60,1,"socket:%d owner:%s output error!",
				pRxTx->socket,
				thread_name(pRxTx->owner_thread));

		dave_mfree(data);
	}

	return dave_true;
}

static inline dave_bool
_base_rxtx_output_normal(dave_bool ct, u8 *dst_ip, u16 dst_port, s32 socket, ORDER_CODE order_id, MBUF *data)
{
	RXTX *pRxTx = NULL;

	pRxTx = _base_rxtx_find_busy(socket);
	if(pRxTx == NULL)
	{
		RTLTRACE(60,1,"can't find the socket:%d! order_id:%x", socket, order_id);
		return dave_false;
	}

	RTDEBUG("order_id:%x data len:%d", order_id, data->len);

	if((data == NULL) || (data->len < SECURE_DATA_MAX_LENGTH))
	{
		if(ct == dave_true)
		{
			return _base_rxtx_output_data_encode(pRxTx, dst_ip, dst_port, RXTX_STACK_CT_DATA_SECRET_VERTYPE, order_id, data);
		}
		else
		{
			return _base_rxtx_output_data_encode(pRxTx, dst_ip, dst_port, RXTX_STACK_REL_SECRET_VERTYPE, order_id, data);
		}
	}
	else
	{
		if(ct == dave_true)
		{
			return _base_rxtx_output_data(pRxTx, dst_ip, dst_port, RXTX_STACK_CT_DATA_VERTYPE, order_id, data);
		}
		else
		{
			return _base_rxtx_output_data(pRxTx, dst_ip, dst_port, RXTX_STACK_REL_VERTYPE, order_id, data);
		}
	}
}

static inline dave_bool
_base_rxtx_output_bin_ct(u8 dst_ip[4], u16 dst_port, s32 socket, CTNote *pNote)
{
	dave_bool ret = dave_false;

	if(pNote != NULL)
	{
		ret = _base_rxtx_output_normal(dave_true, dst_ip, dst_port, socket, pNote->order_id, pNote->data);
		if(ret == dave_true)
		{
			pNote->send_times ++;
		}
	}

	return ret;
}

static inline void
_base_rxtx_receive_fun_process(
	RXTX *pRxTx, stack_receive_fun receive_fun,
	void *param, s32 socket, IPBaseInfo *pIPInfo, FRAMETYPE ver_type,
	ORDER_CODE order_id, ub frame_len, u8 *frame)
{
	if(rxtx_system_rx(pRxTx, order_id, frame_len, frame) == dave_true)
	{
		receive_fun(param, socket, pIPInfo, ver_type, order_id, frame_len, frame);
	}
}

static inline RetCode
_base_rxtx_input_data_decode(RXTX *pRxTx, BinStackMsgHead *message, stack_receive_fun receive_fun)
{
	u8 *package;
	ub package_len;

	package = rxtx_simple_decode_request(message->frame, (ub)(message->frame_len), &package_len);

	if(package != NULL)
	{
		_base_rxtx_receive_fun_process(
			pRxTx, receive_fun,
			pRxTx->param, pRxTx->socket, &(pRxTx->IPInfo), CT_ENCRYPT_DATA_FRAME,
			(ORDER_CODE)(message->order_id), package_len, package);

		rxtx_simple_decode_release(package);

		return RetCode_OK;
	}
	else
	{
		RTLOG("socket:%d decode<order:%x frame_len:%d> failed! %s:%d",
			pRxTx->socket,
			message->order_id, message->frame_len,
			pRxTx->owner_file_name, pRxTx->owner_file_line);

		return RetCode_decode_failed;
	}
}

static inline RetCode
_base_rxtx_input_data(RXTX *pRxTx, BinStackMsgHead *pMessage, stack_receive_fun receive_fun)
{
	u8 *package = pMessage->frame;
	ub package_len;

	RTDEBUG("order_id:%x frame_len:%d", pMessage->order_id, pMessage->frame_len);

	package_len = rxtx_check_crc(pMessage->frame, pMessage->frame_len);
	if(package_len == 0)
	{
		return RetCode_Invalid_data_crc_check;
	}

	_base_rxtx_receive_fun_process(
		pRxTx, receive_fun,
		pRxTx->param, pRxTx->socket, &(pRxTx->IPInfo), CT_DATA_FRAME,
		(ORDER_CODE)(pMessage->order_id), package_len, package);

	return RetCode_OK;
}

static inline RetCode
_base_rxtx_input_rel(RXTX *pRxTx, BinStackMsgHead *pMessage)
{
	stack_receive_fun receive_fun;
	u8 *package = pMessage->frame;
	ub package_len;

	receive_fun = pRxTx->receive_fun;
	if(receive_fun == NULL)
	{
		RTLOG("receive_fun is empty!");
		return RetCode_OK;
	}

	package_len = rxtx_check_crc(pMessage->frame, pMessage->frame_len);
	if(package_len == 0)
	{
		RTLOG("invalid check crc! order_id:%x frame_len:%d frame:%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
			pMessage->order_id, pMessage->frame_len,
			pMessage->frame[0], pMessage->frame[1], pMessage->frame[2],
			pMessage->frame[3], pMessage->frame[4], pMessage->frame[5],
			pMessage->frame[6], pMessage->frame[7], pMessage->frame[8],
			pMessage->frame[9], pMessage->frame[10], pMessage->frame[11]);
		return RetCode_Invalid_data_crc_check;
	}

	_base_rxtx_receive_fun_process(
		pRxTx, receive_fun,
		pRxTx->param, pRxTx->socket, &(pRxTx->IPInfo), REL_FRAME,
		(ORDER_CODE)(pMessage->order_id), package_len, package);

	return RetCode_OK;
}

static inline RetCode
_base_rxtx_input_decode(RXTX *pRxTx, BinStackMsgHead *pMessage)
{
	stack_receive_fun receive_fun;
	RetCode ret;

	RTDEBUG("order_id:%x frame_len:%d", pMessage->order_id, pMessage->frame_len);

	receive_fun = pRxTx->receive_fun;
	if(receive_fun == NULL)
	{
		RTLOG("receive_fun is empty!");
		return RetCode_OK;
	}

	ret = _base_rxtx_input_data_decode(pRxTx, pMessage, receive_fun);
	if(ret != RetCode_OK)
	{
		RTLOG("process error:%s", retstr(ret));
	}

	return ret;
}

static inline RetCode
_base_rxtx_input_ct(dave_bool decode, RXTX *pRxTx, BinStackMsgHead *pMessage)
{
	stack_receive_fun receive_fun;
	RetCode ret;

	receive_fun = pRxTx->receive_fun;
	if(receive_fun == NULL)
	{
		RTLOG("receive_fun is empty!");
		return RetCode_OK;
	}

	if(decode == dave_true)
	{
		ret = _base_rxtx_input_data_decode(pRxTx, pMessage, receive_fun);
	}
	else
	{
		ret = _base_rxtx_input_data(pRxTx, pMessage, receive_fun);
	}

	if(ret != RetCode_OK)
	{
		RTLOG("process error:%s", retstr(ret));
		_base_rxtx_output_sync(pRxTx, pMessage->order_id);
		ret = RetCode_OK;
	}
	else
	{
		_base_rxtx_output_ack(pRxTx, pMessage->order_id);
	}

	return ret;
}

static inline RetCode
_base_rxtx_input_ack(RXTX *pRxTx, BinStackMsgHead *pMessage)
{
	if(rxtx_confirm_transfer_pop(pRxTx->socket, pMessage->order_id) == dave_false)
	{
		RTDEBUG("%s pop failed!", thread_name(pRxTx->owner_thread));
	}
	else
	{
		rxtx_confirm_transfer_out(pRxTx->socket, dave_false);
	}

	return RetCode_OK;
}

static inline RetCode
_base_rxtx_input_sync(RXTX *pRxTx, BinStackMsgHead *pMessage)
{
	rxtx_confirm_transfer_out(pRxTx->socket, dave_true);

	return RetCode_OK;
}

static inline RetCode
_base_rxtx_input_meaningful(FRAMETYPE type, RXTX *pRxTx, BinStackMsgHead *message)
{
	RetCode ret;

	switch(type)
	{
		case REL_FRAME:
				ret = _base_rxtx_input_rel(pRxTx, message);
			break;
		case ENCRYPT_REL_FRAME:
				ret = _base_rxtx_input_decode(pRxTx, message);
			break;
		case CT_ENCRYPT_DATA_FRAME:
				ret = _base_rxtx_input_ct(dave_true, pRxTx, message);
			break;
		case CT_ACK_FRAME:
				ret = _base_rxtx_input_ack(pRxTx, message);
			break;
		case CT_SYNC_FRAME:
				ret = _base_rxtx_input_sync(pRxTx, message);
			break;
		case CT_DATA_FRAME:
				ret = _base_rxtx_input_ct(dave_false, pRxTx, message);
			break;
		default:
				ret = RetCode_OK;
			break;
	}

	return ret;
}

static inline ub
_base_rxtx_input(dave_bool preloading, RXTX *pRxTx, u8 *data_ptr, ub data_len, RetCode *ret)
{
	ub process_index;
	FRAMETYPE type;
	BinStackMsgHead message;
	stack_receive_fun receive_fun;
	dave_bool bad_frame_detected;
	ub bad_frame_index;

	RTDEBUG("%s socket:%d rx_buffer_len:%d",
		thread_name(pRxTx->owner_thread), pRxTx->socket,
		pRxTx->rx_buffer_len);

	*ret = RetCode_OK;

	if((data_ptr == NULL) || (data_len == 0))
	{
		return 0;
	}

	receive_fun = pRxTx->receive_fun;
	if(receive_fun == NULL)
	{
		RTLOG("receive_fun is empty! port:%d data_len:%d <%s:%d>",
			pRxTx->port, data_len,
			pRxTx->owner_file_name, pRxTx->owner_file_line);
		return data_len;
	}

	process_index = 0;

	bad_frame_detected = dave_false;

	bad_frame_index = 0;

	while(process_index < data_len)
	{
		*ret = RetCode_OK;

		type = _base_rxtx_msgtype(&message, &data_ptr[process_index], data_len - process_index);

		switch(type)
		{
			case BAD_FRAME:
					if(bad_frame_detected == dave_false)
					{
						bad_frame_detected = dave_true;
						bad_frame_index = process_index;
					}
					process_index ++;
				break;
			case BAD_LENGTH:
					goto rxtx_input_end;
			case REL_FRAME:
			case ENCRYPT_REL_FRAME:
			case CT_ENCRYPT_DATA_FRAME:
			case CT_ACK_FRAME:
			case CT_SYNC_FRAME:
			case CT_DATA_FRAME:
					if(preloading == dave_false)
					{
						*ret = _base_rxtx_input_meaningful(type, pRxTx, &message);
					}
					process_index += (STACK_HEAD_LENver2 + message.frame_len);
				break;
			default:
					RTLOG("invalid type:%d on index:%d", type, process_index);
					process_index ++;
				break;
		}

		if(*ret != RetCode_OK)
		{
			break;
		}
	}

rxtx_input_end:

	if(bad_frame_detected == dave_true)
	{
		_base_rxtx_has_bad_frame(pRxTx, bad_frame_index, data_ptr, process_index, data_len);

		*ret = RetCode_Invalid_data;
	}

	RTDEBUG("%s socket:%d rx_buffer_len:%d process_index:%d",
		thread_name(pRxTx->owner_thread), pRxTx->socket,
		pRxTx->rx_buffer_len, process_index);

	return process_index;
}

static void
_base_rxtx_event_get(MSGBODY *msg)
{
	RXTXEvent *pEvent = (RXTXEvent *)(((InternalLoop *)(msg->msg_body))->ptr);
	ub process_len;
	RetCode ret;

	process_len = _base_rxtx_input(dave_false, pEvent->pRxTx, pEvent->permutation_ptr, pEvent->permutation_len, &ret);
	if(process_len != pEvent->permutation_len)
	{
		RTLOG("There should be no unprocessed data:%d/%d here!", process_len, pEvent->permutation_len);
	}
	if(ret != RetCode_OK)
	{
		RTLOG("find error:%s", retstr(ret));
	}

	RTDEBUG("%s socket:%d %d/%d serial:%lx",
		thread_name(pEvent->pRxTx->owner_thread), pEvent->pRxTx->socket,
		process_len, pEvent->permutation_len, pEvent->event_serial);

	_base_rxtx_buffer_clean(pEvent->permutation_ptr);

	dave_free(pEvent);
}

static inline void
_base_rxtx_event_set(RXTX *pRxTx, u8 *permutation_ptr, ub permutation_len)
{
	InternalLoop *pLoop = thread_msg(pLoop);
	RXTXEvent *pEvent = dave_malloc(sizeof(RXTXEvent));

	pEvent->pRxTx = pRxTx;
	pEvent->permutation_ptr = permutation_ptr;
	pEvent->permutation_len = permutation_len;
	pEvent->event_serial = dave_os_time_us();

	pLoop->ptr = pEvent;

	RTDEBUG("%s socket:%d %d serial:%lx",
		thread_name(pEvent->pRxTx->owner_thread), pEvent->pRxTx->socket,
		pEvent->permutation_len, pEvent->event_serial);

	id_msg(pRxTx->owner_thread, MSGID_INTERNAL_LOOP, pLoop);
}

static inline RetCode
_base_rxtx_event_process(RXTX *pRxTx)
{
	ub process_len;
	RetCode ret = RetCode_OK;
	dave_bool preloading = dave_true;
	u8 *permutation_ptr;
	ub permutation_len;

	process_len = _base_rxtx_input(preloading, pRxTx, pRxTx->rx_buffer_ptr, pRxTx->rx_buffer_len, &ret);
	if(ret != RetCode_OK)
	{
		RTLOG("%s %d %d/%d", retstr(ret), preloading, process_len, pRxTx->rx_buffer_len);
	}
	if(process_len == 0)
	{
		return RetCode_OK;
	}

	if(preloading == dave_true)
	{
		_base_rxtx_buffer_permutation(&permutation_ptr, &permutation_len, pRxTx, process_len);

		_base_rxtx_event_set(pRxTx, permutation_ptr, permutation_len);
	}
	else
	{
		_base_rxtx_buffer_tidy(pRxTx, process_len);
	}

	return ret;
}

static inline RetCode
_base_rxtx_event_receive(ub *recv_total_length, RXTX *pRxTx, SocketRawEvent *pEvent)
{
	ub rx_buffer_len;
	RetCode ret = RetCode_OK;

	rx_buffer_len = _rxtx_buffer_cfg_length - pRxTx->rx_buffer_len;
	if(rx_buffer_len < RX_TX_BUF_WARRING)
	{
		RTLTRACE(60,1,"rx_buffer too short:%d/%d on %s",
			rx_buffer_len, pRxTx->rx_buffer_len,
			thread_name(pRxTx->owner_thread));
	}

	pEvent->NetInfo.type = pRxTx->type;

	if(dave_os_recv(pEvent->os_socket, &(pEvent->NetInfo),
		&(pRxTx->rx_buffer_ptr[pRxTx->rx_buffer_len]), &rx_buffer_len) == dave_false)
	{
		ret = RetCode_Channel_closed;

		RTTRACE("socket:%d os_socket:%d event:%d domain:%d type:%d ret:%s/%s",
			pEvent->socket, pEvent->os_socket,
			pEvent->event,
			pEvent->NetInfo.domain, pEvent->NetInfo.type,
			retstr(ret), dave_os_errno(NULL));
	}
	else
	{
		if(pRxTx->type == TYPE_SOCK_STREAM)
		{
			pRxTx->IPInfo.protocol = IPProtocol_TCP;

			pRxTx->IPInfo.dst_ip[0] = pEvent->NetInfo.src_ip.ip_addr[0];
			pRxTx->IPInfo.dst_ip[1] = pEvent->NetInfo.src_ip.ip_addr[1];
			pRxTx->IPInfo.dst_ip[2] = pEvent->NetInfo.src_ip.ip_addr[2];
			pRxTx->IPInfo.dst_ip[3] = pEvent->NetInfo.src_ip.ip_addr[3];
			pRxTx->IPInfo.dst_port = pEvent->NetInfo.src_port;
			pRxTx->IPInfo.src_ip[0] = 127;
			pRxTx->IPInfo.src_ip[1] = 0;
			pRxTx->IPInfo.src_ip[2] = 0;
			pRxTx->IPInfo.src_ip[3] = 1;
			pRxTx->IPInfo.src_port = pRxTx->port;
		}
		else
		{
			pRxTx->IPInfo.protocol = IPProtocol_UDP;

			pRxTx->IPInfo.src_ip[0] = pEvent->NetInfo.src_ip.ip_addr[0];
			pRxTx->IPInfo.src_ip[1] = pEvent->NetInfo.src_ip.ip_addr[1];
			pRxTx->IPInfo.src_ip[2] = pEvent->NetInfo.src_ip.ip_addr[2];
			pRxTx->IPInfo.src_ip[3] = pEvent->NetInfo.src_ip.ip_addr[3];
			pRxTx->IPInfo.src_port = pEvent->NetInfo.src_port;
			pRxTx->IPInfo.dst_ip[0] = 127;
			pRxTx->IPInfo.dst_ip[1] = 0;
			pRxTx->IPInfo.dst_ip[2] = 0;
			pRxTx->IPInfo.dst_ip[3] = 1;
			pRxTx->IPInfo.dst_port = pRxTx->port;
		}

		RTDEBUG("%s->%s len:%d",
			ipv4str(pRxTx->IPInfo.src_ip, pRxTx->IPInfo.src_port),
			ipv4str2(pRxTx->IPInfo.dst_ip, pRxTx->IPInfo.dst_port),
			rx_buffer_len);
	}

	if(ret == RetCode_OK)
	{
		*recv_total_length += rx_buffer_len;
		pRxTx->rx_buffer_len += rx_buffer_len;
		if(pRxTx->rx_buffer_len < _rxtx_buffer_cfg_length)
		{
			pRxTx->rx_buffer_ptr[pRxTx->rx_buffer_len] = '\0';
		}
	}

	return ret;
}

static inline RetCode
_base_rxtx_event_action(ub *recv_total_length, SocketRawEvent *pEvent, stack_receive_fun receive_fun, void *param)
{
	RXTX *pRxTx = NULL;
	ub safe_counter;
	ub backup_rx_buf_len;
	RetCode ret = RetCode_OK;

	pRxTx = _base_rxtx_find_busy(pEvent->socket);
	if(pRxTx == NULL)
	{
		RTLOG("socket:%d close or not ready!", pEvent->socket);
		dave_mfree(pEvent->data);
		return RetCode_lost_link;
	}
	pRxTx->receive_fun = receive_fun;
	pRxTx->param = param;

	safe_counter = 0;

	SAFECODEidlev1(pRxTx->opt_pv, {

		_base_rxtx_buffer_malloc(pRxTx);

		if((pRxTx->socket != INVALID_SOCKET_ID) && (pRxTx->socket >= 0))
		{
			while((pRxTx->rx_buffer_ptr != NULL) && ((++ safe_counter) <= RECV_COUNTER_MAX))
			{
				backup_rx_buf_len = pRxTx->rx_buffer_len;

				ret = _base_rxtx_event_receive(recv_total_length, pRxTx, pEvent);
				if(ret != RetCode_OK)
				{
					break;
				}

				if(pRxTx->rx_buffer_len > backup_rx_buf_len)
				{
					ret = _base_rxtx_event_process(pRxTx);
					if(ret != RetCode_OK)
					{
						break;
					}
				}
				else
				{
					ret = RetCode_OK;
					break;
				}
			}
		}

		_base_rxtx_buffer_free(pRxTx);

	} );

	RTDEBUG("socket:%d port:%d ret:%s rx_buffer_len:%d safe_counter:%d",
		pRxTx->socket, pRxTx->port, retstr(ret),
		pRxTx->rx_buffer_len,
		safe_counter);

	if((safe_counter == 0) || (safe_counter >= (RECV_COUNTER_MAX / 2)))
	{
		_base_rxtx_maybe_has_data(pRxTx, pEvent);
	}

	dave_mfree(pEvent->data);

	return ret;
}

static inline void
_base_rxtx_event_notify_recv_length(s32 socket, ub recv_total_length, void *ptr)
{
	SocketNotify *pNotify = thread_msg(pNotify);

	pNotify->socket = socket;
	pNotify->notify = SOCKETINFO_RAW_EVENT_RECV_LENGTH;
	pNotify->data = recv_total_length;
	pNotify->ptr = ptr;

	id_msg(_socket_thread, SOCKET_NOTIFY, pNotify);
}

static inline void
_base_rxtx_safe_build(RXTX *pRxTx, SOCTYPE type, s32 socket, u16 port, s8 *file, ub line)
{
	SAFECODEv1(pRxTx->opt_pv, {

		_base_rxtx_reset(pRxTx);
		
		pRxTx->type = type;
		pRxTx->socket = socket;
		pRxTx->port = port;
		pRxTx->owner_thread = self();
		pRxTx->owner_file_name = file;
		pRxTx->owner_file_line = line;

	} );
}

static inline void
_base_rxtx_safe_clean(RXTX *pRxTx, s8 *file, ub line)
{
	u8 *rx_buffer_ptr = NULL;

	SAFECODEv1(pRxTx->opt_pv, {

		rx_buffer_ptr = pRxTx->rx_buffer_ptr;

		_base_rxtx_reset(pRxTx);

	} );

	if(rx_buffer_ptr != NULL)
	{
		dave_free(rx_buffer_ptr);
	}
}

static inline void
_base_rxtx_show(void)
{
	ub rxtx_index;

	for(rxtx_index=0; rxtx_index<RXTX_MAX; rxtx_index++)
	{
		RTLOG("index:%d socket:%d port:%d thread:%s %s:%d",
			rxtx_index,
			_rx_tx[rxtx_index].socket, _rx_tx[rxtx_index].port,
			thread_name(_rx_tx[rxtx_index].owner_thread),
			_rx_tx[rxtx_index].owner_file_name,
			_rx_tx[rxtx_index].owner_file_line);
	}
}

static void
_base_rxtx_update_buffer_length(void)
{
	_rxtx_buffer_cfg_length = cfg_get_ub(RXTX_BUFFER_CFG_NAME, RX_TX_BUF_SETUP);
	if(_rxtx_buffer_cfg_length < RX_TX_BUF_MIN)
	{
		_rxtx_buffer_cfg_length = RX_TX_BUF_MIN;
	}
}

// =====================================================================

void
base_rxtx_init(void)
{
	ub rxtx_index;

	_socket_thread = thread_id(SOCKET_THREAD_NAME);

	t_lock_reset(&_opt_pv);

	_base_rxtx_update_buffer_length();

	for(rxtx_index=0; rxtx_index<RXTX_MAX; rxtx_index++)
	{
		dave_memset(&_rx_tx[rxtx_index], 0x00, sizeof(RXTX));

		_base_rxtx_reset(&_rx_tx[rxtx_index]);

		_rx_tx[rxtx_index].rxtx_index = rxtx_index;

		t_lock_reset(&_rx_tx[rxtx_index].opt_pv);
	}

	rxtx_confirm_transfer_init(_base_rxtx_output_bin_ct);
}

void
base_rxtx_exit(void)
{
	rxtx_confirm_transfer_exit();
}

dave_bool
__base_rxtx_build__(SOCTYPE type, s32 socket, u16 port, s8 *file, ub line)
{
	RXTX *pRxTx;
	dave_bool ret = dave_false;

	_socket_thread = thread_id(SOCKET_THREAD_NAME);

	SAFECODEv1(_opt_pv, {

		pRxTx = _base_rxtx_find_free(socket);
		if(pRxTx == NULL)
		{
			RTABNOR("build rxtx socket:%d fail! %s:%d", socket, file, line);
			ret = dave_false;
		}
		else
		{
			_base_rxtx_safe_build(pRxTx, type, socket, port, file, line);

			ret = dave_true;

			rxtx_confirm_transfer_clean(socket);
		}

	} );

	if(ret == dave_false)
	{
		_base_rxtx_show();
	}
	else
	{
		reg_msg(MSGID_INTERNAL_LOOP, _base_rxtx_event_get);

		rxtx_system_no_crc_tx(pRxTx);
	}

	return ret;
}

void
__base_rxtx_clean__(s32 socket, s8 *file, ub line)
{
	RXTX *pRxTx;

	SAFECODEv1(_opt_pv, {

		rxtx_confirm_transfer_clean(socket);

		pRxTx = _base_rxtx_find_busy(socket);
		if(pRxTx != NULL)
		{
			_base_rxtx_safe_clean(pRxTx, file, line);
		}
		else
		{
			RTTRACE("socket:%d not here! <%s:%d>", socket, file, line);
		}

	} );
}

dave_bool
base_rxtx_writes(s32 socket, ORDER_CODE order_id, MBUF *data)
{
	dave_bool ret;

	ret = _base_rxtx_output_normal(dave_false, NULL, 0, socket, order_id, data);
	if(ret == dave_false)
	{
		dave_mfree(data);
	}

	return ret;
}

dave_bool
base_rxtx_send_ct(u8 dst_ip[4], u16 dst_port, s32 socket, ORDER_CODE order_id, MBUF *data)
{
	dave_bool ret;

	ret = rxtx_confirm_transfer_push(dst_ip, dst_port, socket, order_id, data);
	if(ret == dave_false)
	{
		RTABNOR("push failed:%x", order_id);
		dave_mfree(data);
	}

	rxtx_confirm_transfer_out(socket, dave_false);

	return ret;
}

dave_bool
base_rxtx_send(u8 dst_ip[4], u16 dst_port, s32 socket, ORDER_CODE order_id, MBUF *data)
{
	dave_bool ret;

	ret = _base_rxtx_output_normal(dave_false, dst_ip, dst_port, socket, order_id, data);
	if(ret == dave_false)
	{
		dave_mfree(data);
	}

	return ret;
}

RetCode
base_rxtx_event(SocketRawEvent *pEvent, stack_receive_fun receive_fun, void *param)
{
	RetCode ret;
	ub recv_total_length = 0;

	ret = _base_rxtx_event_action(&recv_total_length, pEvent, receive_fun, param);

	_base_rxtx_event_notify_recv_length(pEvent->socket, recv_total_length, pEvent->ptr);

	return ret;
}

#endif


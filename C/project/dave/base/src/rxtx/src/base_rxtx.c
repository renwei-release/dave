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
#include "rxtx_param.h"
#include "rxtx_tools.h"
#include "rxtx_log.h"

#define SECURE_DATA_MAX_LENGTH 32
#define RECV_COUNTER_MAX (256)
#define MAYBE_HAS_DATA_COUNTER_MAX (1)

static ThreadId _socket_thread = INVALID_THREAD_ID;
static TLock _opt_pv;
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

static inline FRAMETYPE
_base_rxtx_to_type(u8 data_type)
{
	FRAMETYPE type;

	switch(data_type)
	{
		case RXTX_STACK_REL_VERTYPE_ver2:
				type = REL_FRAME;
			break;
		case RXTX_STACK_REL_SECRET_VERTYPE_ver2:
				type = ENCRYPT_REL_FRAME;
			break;
		case RXTX_STACK_CT_DATA_SECRET_VERTYPE_ver2:
				type = CT_ENCRYPT_DATA_FRAME;
			break;
		case RXTX_STACK_CT_ACK_VERTYPE_ver2:
				type = CT_ACK_FRAME;
			break;
		case RXTX_STACK_CT_SYNC_VERTYPE_ver2:
				type = CT_SYNC_FRAME;
			break;
		case RXTX_STACK_CT_DATA_VERTYPE_ver2:
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
		case RXTX_STACK_REL_VERTYPE_ver2:
		case RXTX_STACK_REL_SECRET_VERTYPE_ver2:
		case RXTX_STACK_CT_DATA_SECRET_VERTYPE_ver2:
		case RXTX_STACK_CT_ACK_VERTYPE_ver2:
		case RXTX_STACK_CT_SYNC_VERTYPE_ver2:
		case RXTX_STACK_CT_DATA_VERTYPE_ver2:
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
_base_rxtx_has_bad_frame(RXTX *pRxTx, dave_bool bad_frame, ub bad_frame_index, u8 *data, ub data_len)
{
	s8 *bad_data_file;
	ub bad_data_length, bad_data_index, data_index;
	s8 file_name[256];
	DateStruct file_time;

	if(bad_frame == dave_true)
	{
		RTLOG("thread:%s socket:%d has bad frame start on:%d, data len:%d (%s:%d)",
			thread_name(pRxTx->owner_thread), pRxTx->socket,
			bad_frame_index, data_len,
			pRxTx->owner_file_name, pRxTx->owner_file_line);

		bad_data_length = data_len * 16 + 16;

		bad_data_file = dave_malloc(bad_data_length);

		for(data_index=0,bad_data_index=0; data_index<data_len; data_index++)
		{
			bad_data_index += dave_snprintf(&bad_data_file[bad_data_index], bad_data_length-bad_data_index, "0x%02x, ", data[data_index]);
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
}

static inline ub
_base_rxtx_build_head(u8 *head_data, u8 ver_type, ORDER_CODE order_id, ub data_length)
{
	ub head_index = 0;

	ver_type &= 0x0f;
	ver_type |= (RXTX_STACK_VERSION2<<4);

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

	_base_rxtx_output(RXTX_STACK_CT_ACK_VERTYPE_ver2, pRxTx, pRxTx->IPInfo.src_ip, pRxTx->IPInfo.src_port, order_id, NULL);
}

static inline void
_base_rxtx_output_sync(RXTX *pRxTx, ORDER_CODE order_id)
{
	_base_rxtx_output(RXTX_STACK_CT_SYNC_VERTYPE_ver2, pRxTx, pRxTx->IPInfo.src_ip, pRxTx->IPInfo.src_port, order_id, NULL);
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

	crc->tot_len = crc->len = rxtx_build_crc_v2(data, dave_mptr(crc), crc->len);

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

	if(data->len < SECURE_DATA_MAX_LENGTH)
	{
		if(ct == dave_true)
		{
			return _base_rxtx_output_data_encode(pRxTx, dst_ip, dst_port, RXTX_STACK_CT_DATA_SECRET_VERTYPE_ver2, order_id, data);
		}
		else
		{
			return _base_rxtx_output_data_encode(pRxTx, dst_ip, dst_port, RXTX_STACK_REL_SECRET_VERTYPE_ver2, order_id, data);
		}
	}
	else
	{
		if(ct == dave_true)
		{
			return _base_rxtx_output_data(pRxTx, dst_ip, dst_port, RXTX_STACK_CT_DATA_VERTYPE_ver2, order_id, data);
		}
		else
		{
			return _base_rxtx_output_data(pRxTx, dst_ip, dst_port, RXTX_STACK_REL_VERTYPE_ver2, order_id, data);
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

static inline ErrCode
_base_rxtx_input_data_decode(RXTX *pRxTx, BinStackMsgHead *message, stack_receive_fun receive_fun)
{
	u8 *package;
	ub package_len;

	package = rxtx_simple_decode_request(message->frame, (ub)(message->frame_len), &package_len);

	if(package != NULL)
	{
		receive_fun(pRxTx->param, pRxTx->socket, &(pRxTx->IPInfo), CT_ENCRYPT_DATA_FRAME, (ORDER_CODE)(message->order_id), package_len, package);

		rxtx_simple_decode_release(package);

		return ERRCODE_OK;
	}
	else
	{
		RTLOG("socket:%d decode<order:%x frame_len:%d> failed! %s:%d",
			pRxTx->socket,
			message->order_id, message->frame_len,
			pRxTx->owner_file_name, pRxTx->owner_file_line);

		return ERRCODE_decode_failed;
	}
}

static inline ErrCode
_base_rxtx_input_data(RXTX *pRxTx, BinStackMsgHead *pMessage, stack_receive_fun receive_fun)
{
	u8 *package = pMessage->frame;
	ub package_len;

	RTDEBUG("order_id:%x frame_len:%d", pMessage->order_id, pMessage->frame_len);

	package_len = rxtx_check_crc(pMessage->frame, pMessage->frame_len);
	if(package_len == 0)
	{
		return ERRCODE_Invalid_data_crc_check;
	}

	receive_fun(pRxTx->param, pRxTx->socket, &(pRxTx->IPInfo), CT_DATA_FRAME, (ORDER_CODE)(pMessage->order_id), package_len, package);

	return ERRCODE_OK;
}

static inline ub
_base_rxtx_input_rel(RXTX *pRxTx, BinStackMsgHead *pMessage, ErrCode *ret)
{
	stack_receive_fun receive_fun;
	u8 *package = pMessage->frame;
	ub package_len;

	*ret = ERRCODE_OK;

	receive_fun = pRxTx->receive_fun;
	if(receive_fun == NULL)
	{
		RTLOG("receive_fun is empty!");
		return (STACK_HEAD_LENver2 + pMessage->frame_len);
	}

	package_len = rxtx_check_crc(pMessage->frame, pMessage->frame_len);
	if(package_len == 0)
	{
		*ret = ERRCODE_Invalid_data_crc_check;
		RTLOG("invalid check crc! order_id:%x frame_len:%d frame:%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
			pMessage->order_id, pMessage->frame_len,
			pMessage->frame[0], pMessage->frame[1], pMessage->frame[2],
			pMessage->frame[3], pMessage->frame[4], pMessage->frame[5],
			pMessage->frame[6], pMessage->frame[7], pMessage->frame[8],
			pMessage->frame[9], pMessage->frame[10], pMessage->frame[11]);
		return 1;
	}

	receive_fun(pRxTx->param, pRxTx->socket, &(pRxTx->IPInfo), REL_FRAME, (ORDER_CODE)(pMessage->order_id), package_len, package);

	return (STACK_HEAD_LENver2 + pMessage->frame_len);
}

static inline ub
_base_rxtx_input_decode(RXTX *pRxTx, BinStackMsgHead *pMessage, ErrCode *ret)
{
	stack_receive_fun receive_fun;

	RTDEBUG("order_id:%x frame_len:%d", pMessage->order_id, pMessage->frame_len);

	*ret = ERRCODE_OK;

	receive_fun = pRxTx->receive_fun;
	if(receive_fun == NULL)
	{
		RTLOG("receive_fun is empty!");
		return (STACK_HEAD_LENver2 + pMessage->frame_len);
	}

	*ret = _base_rxtx_input_data_decode(pRxTx, pMessage, receive_fun);
	if(*ret != ERRCODE_OK)
	{
		RTLOG("process error:%s", t_a2b_errorstr(*ret));
	}

	if(*ret == ERRCODE_OK)
	{
		return (STACK_HEAD_LENver2 + pMessage->frame_len);
	}
	else
	{
		RTLOG("invalid check crc! ret:%s", t_a2b_errorstr(*ret));
		return 1;
	}
}

static inline ub
_base_rxtx_input_ct(dave_bool decode, RXTX *pRxTx, BinStackMsgHead *pMessage, ub process_index, ub data_len, ErrCode *ret)
{
	stack_receive_fun receive_fun;

	RTDEBUG("decode:%d data_len:%d", decode, data_len);

	*ret = ERRCODE_OK;

	receive_fun = pRxTx->receive_fun;
	if(receive_fun == NULL)
	{
		RTLOG("receive_fun is empty!");
		return (STACK_HEAD_LENver2 + pMessage->frame_len);
	}

	if(decode == dave_true)
	{
		*ret = _base_rxtx_input_data_decode(pRxTx, pMessage, receive_fun);
	}
	else
	{
		*ret = _base_rxtx_input_data(pRxTx, pMessage, receive_fun);
	}

	if(*ret != ERRCODE_OK)
	{
		RTLOG("process error:%s data len:%d/%d", t_a2b_errorstr(*ret), process_index, data_len);

		_base_rxtx_output_sync(pRxTx, pMessage->order_id);

		*ret = ERRCODE_OK;
	}
	else
	{
		_base_rxtx_output_ack(pRxTx, pMessage->order_id);
	}

	if(*ret == ERRCODE_OK)
	{
		return (STACK_HEAD_LENver2 + pMessage->frame_len);
	}
	else
	{
		RTLOG("invalid check crc! ret:%s", t_a2b_errorstr(*ret));
		return 1;
	}
}

static inline ub
_base_rxtx_input_ack(RXTX *pRxTx, BinStackMsgHead *pMessage, ErrCode *ret)
{
	*ret = ERRCODE_OK;

	if(rxtx_confirm_transfer_pop(pRxTx->socket, pMessage->order_id) == dave_false)
	{
		RTDEBUG("%s pop failed!", thread_name(pRxTx->owner_thread));
	}
	else
	{
		rxtx_confirm_transfer_out(pRxTx->socket, dave_false);
	}

	return STACK_HEAD_LENver2;
}

static inline ub
_base_rxtx_input_sync(RXTX *pRxTx, BinStackMsgHead *pMessage, ErrCode *ret)
{
	*ret = ERRCODE_OK;

	rxtx_confirm_transfer_out(pRxTx->socket, dave_true);

	return STACK_HEAD_LENver2;
}

static inline ub
_base_rxtx_input(RXTX *pRxTx, u8 *data, ub data_len, ErrCode *result)
{
	ub process_index;
	FRAMETYPE type;
	BinStackMsgHead message;
	stack_receive_fun receive_fun;
	ErrCode ret;
	dave_bool bad_frame_detected;
	ub bad_frame_index;

	RTDEBUG("%s socket:%d rx_buffer_len:%d",
		thread_name(pRxTx->owner_thread), pRxTx->socket,
		pRxTx->rx_buffer_len);

	*result = ERRCODE_OK;

	if((data == NULL) || (data_len == 0))
	{
		return 0;
	}

	receive_fun = pRxTx->receive_fun;
	if(receive_fun == NULL)
	{
		RTLOG("receive_fun is empty!");
		return data_len;
	}

	process_index = 0;

	bad_frame_detected = dave_false;

	bad_frame_index = 0;

	while(process_index < data_len)
	{
		ret = ERRCODE_OK;

		type = _base_rxtx_msgtype(&message, &data[process_index], data_len - process_index);

		RTDEBUG("ver_type:%x type:%d frame_len:%d order_id:%x data:%d/%d",
			message.ver_type, type, message.frame_len, message.order_id,
			process_index, data_len);

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
					process_index += _base_rxtx_input_rel(pRxTx, &message, &ret);
				break;
			case ENCRYPT_REL_FRAME:
					process_index += _base_rxtx_input_decode(pRxTx, &message, &ret);
				break;
			case CT_ENCRYPT_DATA_FRAME:
					process_index += _base_rxtx_input_ct(dave_true, pRxTx, &message, process_index, data_len, &ret);
				break;
			case CT_ACK_FRAME:
					process_index += _base_rxtx_input_ack(pRxTx, &message, &ret);
				break;
			case CT_SYNC_FRAME:
					process_index += _base_rxtx_input_sync(pRxTx, &message, &ret);
				break;
			case CT_DATA_FRAME:
					process_index += _base_rxtx_input_ct(dave_false, pRxTx, &message, process_index, data_len, &ret);
				break;
			default:
					RTLOG("invalid type:%d on index:%d", type, process_index);
					process_index ++;
				break;
		}

		if(ret != ERRCODE_OK)
		{
			*result = ret;

			break;
		}
	}

rxtx_input_end:

	if(bad_frame_detected == dave_true)
	{
		_base_rxtx_has_bad_frame(pRxTx, bad_frame_detected, bad_frame_index, data, data_len);

		*result = ERRCODE_Invalid_data;
	}

	RTDEBUG("%s socket:%d rx_buffer_len:%d process_index:%d",
		thread_name(pRxTx->owner_thread), pRxTx->socket,
		pRxTx->rx_buffer_len, process_index);

	return process_index;
}

static inline ErrCode
_base_rxtx_read_process(RXTX *pRxTx, u8 *data, ub data_len)
{
	ub process_len;
	ErrCode ret = ERRCODE_OK;

	if((pRxTx->rx_buffer_len == 0) && (data != NULL) && (data_len > 0))
	{
		process_len = _base_rxtx_input(pRxTx, data, data_len, &ret);
		if(process_len < data_len)
		{
			pRxTx->rx_buffer_len = data_len - process_len;
			if(pRxTx->rx_buffer_len > RX_TX_BUF_MAX)
			{
				RTABNOR("data buffer overflow! %d/%d", pRxTx->rx_buffer_len, data_len);

				pRxTx->rx_buffer_len = 0;
				ret = ERRCODE_data_overflow;
			}

			dave_memcpy(pRxTx->rx_buffer_ptr, &data[process_len], pRxTx->rx_buffer_len);

			if(pRxTx->rx_buffer_len < RX_TX_BUF_MAX)
			{
				pRxTx->rx_buffer_ptr[pRxTx->rx_buffer_len] = '\0';
			}
		}
	}
	else
	{
		if((pRxTx->rx_buffer_len + data_len) > RX_TX_BUF_MAX)
		{
			RTABNOR("data buffer overflow! <%d/%d>",
				pRxTx->rx_buffer_len, data_len);

			pRxTx->rx_buffer_len = 0;
			ret = ERRCODE_data_overflow;
		}

		if((data != NULL) && (data_len > 0))
		{
			dave_memcpy(&(pRxTx->rx_buffer_ptr[pRxTx->rx_buffer_len]), data, data_len);

			pRxTx->rx_buffer_len += data_len;
		}

		if(pRxTx->rx_buffer_len < RX_TX_BUF_MAX)
		{
			pRxTx->rx_buffer_ptr[pRxTx->rx_buffer_len] = '\0';
		}

		process_len = _base_rxtx_input(pRxTx, pRxTx->rx_buffer_ptr, pRxTx->rx_buffer_len, &ret);

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

			if(pRxTx->rx_buffer_len < RX_TX_BUF_MAX)
			{
				pRxTx->rx_buffer_ptr[pRxTx->rx_buffer_len] = '\0';
			}
		}
	}

	return ret;
}

static inline ErrCode
_base_rxtx_read(SocketRead *pRead, RXTX *pRxTx, stack_receive_fun result_fun, void *param)
{
	u8 *data;
	ub data_len;

	if(pRead->data != NULL)
	{
		data = (u8 *)(pRead->data->payload);
		data_len = pRead->data->tot_len;
	}
	else
	{
		data = NULL;
		data_len = 0;
	}

	pRxTx->receive_fun = result_fun;
	pRxTx->param = param;

	RTDEBUG("%s read data:%d/%d",
		thread_name(pRxTx->owner_thread),
		pRead->data_len,
		pRxTx->rx_buffer_len);

	pRxTx->IPInfo = pRead->IPInfo;

	return _base_rxtx_read_process(pRxTx, data, data_len);
}

static inline void
_base_rxtx_safe_build(RXTX *pRxTx, SOCTYPE type, s32 socket, u16 port, s8 *file, ub line)
{
	SAFEZONEv3(pRxTx->opt_pv, {

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

	SAFEZONEv3(pRxTx->opt_pv, {

		rx_buffer_ptr = pRxTx->rx_buffer_ptr;

		_base_rxtx_reset(pRxTx);

	} );

	if(rx_buffer_ptr != NULL)
	{
		dave_free(rx_buffer_ptr);
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

	write_nmsg(pRxTx->owner_thread, SOCKET_RAW_EVENT, pEvent, 128);
}

static inline ErrCode
_base_rxtx_event_receive(ub *recv_total_length, RXTX *pRxTx, SocketRawEvent *pEvent, SocketRead *pRead)
{
	ub rx_buffer_len;
	ErrCode ret = ERRCODE_OK;

	rx_buffer_len = RX_TX_BUF_MAX - pRxTx->rx_buffer_len;
	if(rx_buffer_len < RX_TX_BUF_MIN)
	{
		RTLTRACE(60,1,"rx_buffer too short:%d/%d on %s",
			rx_buffer_len, pRxTx->rx_buffer_len,
			thread_name(pRxTx->owner_thread));
	}

	pEvent->NetInfo.type = pRxTx->type;

	if(dave_os_recv(pEvent->os_socket, &(pEvent->NetInfo),
		&(pRxTx->rx_buffer_ptr[pRxTx->rx_buffer_len]), &rx_buffer_len) == dave_false)
	{
		ret = ERRCODE_Channel_closed;

		RTTRACE("socket:%d os_socket:%d event:%d domain:%d type:%d ret:%s/%s",
			pEvent->socket, pEvent->os_socket,
			pEvent->event,
			pEvent->NetInfo.domain, pEvent->NetInfo.type,
			t_a2b_errorstr(ret), dave_os_errno(NULL));
	}
	else
	{
		if(pRxTx->type == TYPE_SOCK_STREAM)
		{
			pRead->IPInfo.protocol = IPProtocol_TCP;

			pRead->IPInfo.dst_ip[0] = pEvent->NetInfo.src_ip.ip_addr[0];
			pRead->IPInfo.dst_ip[1] = pEvent->NetInfo.src_ip.ip_addr[1];
			pRead->IPInfo.dst_ip[2] = pEvent->NetInfo.src_ip.ip_addr[2];
			pRead->IPInfo.dst_ip[3] = pEvent->NetInfo.src_ip.ip_addr[3];
			pRead->IPInfo.dst_port = pEvent->NetInfo.src_port;
			pRead->IPInfo.src_ip[0] = 127;
			pRead->IPInfo.src_ip[1] = 0;
			pRead->IPInfo.src_ip[2] = 0;
			pRead->IPInfo.src_ip[3] = 1;
			pRead->IPInfo.src_port = pRxTx->port;
		}
		else
		{
			pRead->IPInfo.protocol = IPProtocol_UDP;

			pRead->IPInfo.src_ip[0] = pEvent->NetInfo.src_ip.ip_addr[0];
			pRead->IPInfo.src_ip[1] = pEvent->NetInfo.src_ip.ip_addr[1];
			pRead->IPInfo.src_ip[2] = pEvent->NetInfo.src_ip.ip_addr[2];
			pRead->IPInfo.src_ip[3] = pEvent->NetInfo.src_ip.ip_addr[3];
			pRead->IPInfo.src_port = pEvent->NetInfo.src_port;
			pRead->IPInfo.dst_ip[0] = 127;
			pRead->IPInfo.dst_ip[1] = 0;
			pRead->IPInfo.dst_ip[2] = 0;
			pRead->IPInfo.dst_ip[3] = 1;
			pRead->IPInfo.dst_port = pRxTx->port;
		}

		RTDEBUG("%s->%s len:%d",
			ipv4str(pRead->IPInfo.src_ip, pRead->IPInfo.src_port),
			ipv4str2(pRead->IPInfo.dst_ip, pRead->IPInfo.dst_port),
			rx_buffer_len);
	}

	if(ret == ERRCODE_OK)
	{
		*recv_total_length += rx_buffer_len;

		pRxTx->rx_buffer_len += rx_buffer_len;	
	}

	return ret;
}

static inline void
_base_rxtx_buffer_malloc(RXTX *pRxTx)
{
	if(pRxTx->rx_buffer_ptr == NULL)
	{
		pRxTx->rx_buffer_ptr = dave_malloc(RX_TX_BUF_MAX);
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

static inline ErrCode
_base_rxtx_input_action(SocketRead *pRead, stack_receive_fun result_fun, void *param)
{
	ErrCode ret = ERRCODE_msg_competition_for_resources;
	RXTX *pRxTx;

	pRxTx = _base_rxtx_find_busy(pRead->socket);
	if(pRxTx == NULL)
	{
		RTLOG("socket:%d close! <%d>", pRead->socket, pRead->data_len);
		dave_mfree(pRead->data);
		return ERRCODE_lost_link;
	}

	SAFEZONEv3(pRxTx->opt_pv, {

		_base_rxtx_buffer_malloc(pRxTx);

		if((pRxTx->socket != INVALID_SOCKET_ID) && (pRxTx->socket >= 0))
		{
			ret = _base_rxtx_read(pRead, pRxTx, result_fun, param);
		}
		else
		{
			RTLOG("socket:%d close! <%d>", pRead->socket, pRead->data_len);
			ret = ERRCODE_lost_link;
		}

		_base_rxtx_buffer_free(pRxTx);

	} );

	if(ret != ERRCODE_OK)
	{
		RTABNOR("%s socket:%d input data failed:%s! data_len:%d rx_buffer_len:%d",
			thread_name(self()), pRead->socket, t_a2b_errorstr(ret),
			pRead->data_len, pRxTx->rx_buffer_len);
	}

	dave_mfree(pRead->data);

	return ret;
}

static inline ErrCode
_base_rxtx_event_action(ub *recv_total_length, SocketRawEvent *pEvent, stack_receive_fun result_fun, void *param)
{
	RXTX *pRxTx = NULL;
	SocketRead read;
	ub safe_counter;
	ub backup_rx_buf_len;
	ErrCode ret = ERRCODE_OK;

	pRxTx = _base_rxtx_find_busy(pEvent->socket);
	if(pRxTx == NULL)
	{
		RTLOG("socket:%d close or not ready!", pEvent->socket);
		dave_mfree(pEvent->data);
		return ERRCODE_lost_link;
	}

	dave_memset(&read, 0x00, sizeof(SocketRead));

	read.socket = pRxTx->socket;

	safe_counter = 0;

	SAFEZONEidlev3(pRxTx->opt_pv, {

		_base_rxtx_buffer_malloc(pRxTx);

		if((pRxTx->socket != INVALID_SOCKET_ID) && (pRxTx->socket >= 0))
		{
			while((++ safe_counter) <= RECV_COUNTER_MAX)
			{
				backup_rx_buf_len = pRxTx->rx_buffer_len;

				ret = _base_rxtx_event_receive(recv_total_length, pRxTx, pEvent, &read);
				if(ret != ERRCODE_OK)
				{
					break;
				}

				RTDEBUG("%s socket:%d rx_buffer_len:%d",
					thread_name(pRxTx->owner_thread),
					pRxTx->socket,
					pRxTx->rx_buffer_len);

				if(pRxTx->rx_buffer_len > backup_rx_buf_len)
				{
					ret = _base_rxtx_read(&read, pRxTx, result_fun, param);
					if(ret != ERRCODE_OK)
					{
						break;
					}
				}
				else
				{
					ret = ERRCODE_OK;
					break;
				}
			}
		}

		_base_rxtx_buffer_free(pRxTx);

	} );

	RTDEBUG("socket:%d port:%d ret:%s rx_buffer_len:%d safe_counter:%d",
		pRxTx->socket, pRxTx->port, t_a2b_errorstr(ret),
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

	write_msg(_socket_thread, SOCKET_NOTIFY, pNotify);
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

// =====================================================================

void
base_rxtx_init(void)
{
	ub rxtx_index;

	_socket_thread = thread_id(SOCKET_THREAD_NAME);

	t_lock_reset(&_opt_pv);

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

	SAFEZONEv3(_opt_pv, {

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

	return ret;
}

void
__base_rxtx_clean__(s32 socket, s8 *file, ub line)
{
	RXTX *pRxTx;

	SAFEZONEv3(_opt_pv, {

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

	RTDEBUG("socket:%d", socket);
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

	return dave_true;
}

dave_bool
base_rxtx_send_ct(u8 dst_ip[4], u16 dst_port, s32 socket, ORDER_CODE order_id, MBUF *data)
{
	RTDEBUG("socket:%d order_id:%x length:%d", socket, order_id, data->len);

	if(rxtx_confirm_transfer_push(dst_ip, dst_port, socket, order_id, data) == dave_false)
	{
		RTABNOR("push failed:%x", order_id);
		dave_mfree(data);
	}

	rxtx_confirm_transfer_out(socket, dave_false);

	return dave_true;
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

	return dave_true;
}

ErrCode
base_rxtx_input(SocketRead *pRead, stack_receive_fun result_fun, void *param)
{
	return _base_rxtx_input_action(pRead, result_fun, param);
}

ErrCode
base_rxtx_event(SocketRawEvent *pEvent, stack_receive_fun result_fun, void *param)
{
	ErrCode ret;
	ub recv_total_length = 0;

	if(pEvent->data == NULL)
	{
		ret = _base_rxtx_event_action(&recv_total_length, pEvent, result_fun, param);
	}
	else
	{
		SocketRead read;

		read.socket = pEvent->socket;
		T_NetToIPInfo(&(read.IPInfo), &(pEvent->NetInfo));
		read.data_len = pEvent->data->len;
		read.data = pEvent->data;

		recv_total_length += pEvent->data->len;

		ret = _base_rxtx_input_action(&read, result_fun, param);
	}

	_base_rxtx_event_notify_recv_length(pEvent->socket, recv_total_length, pEvent->ptr);

	return ret;
}

#endif


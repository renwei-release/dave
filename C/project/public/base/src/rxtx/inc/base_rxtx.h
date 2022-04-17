/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_RXTX_H__
#define __BASE_RXTX_H__
#include "base_macro.h"
#include "dave_base.h"

typedef enum {
	BAD_FRAME = 0,
	BAD_LENGTH,
	REL_FRAME,
	ENCRYPT_REL_FRAME,
	CT_ENCRYPT_DATA_FRAME,
	CT_ACK_FRAME,
	CT_SYNC_FRAME,
	CT_DATA_FRAME,
	
	FRAMETYPE_MAX
} FRAMETYPE;

typedef enum {
	ORDER_CODE_BEGIN						= 0x0100,

	/* log function order code */
	ORDER_CODE_LOG_RECORD					= 0x0102,

	ORDER_CODE_RUN_INTERNAL_MSG_REQ 		= 0x0114,
	ORDER_CODE_RUN_INTERNAL_MSG_RSP 		= 0x0115,

	/* sync function order code */
	ORDER_CODE_MY_VERNO						= 0x0201,
	ORDER_CODE_HEARTBEAT					= 0x0202,
	ORDER_CODE_MODULE_VERNO					= 0x0203,
	ORDER_CODE_SYNC_THREAD_NAME_REQ			= 0x0204,
	ORDER_CODE_SYNC_THREAD_NAME_RSP			= 0x0205,
	ORDER_CODE_ADD_REMOTE_THREAD_REQ		= 0x0206,
	ORDER_CODE_ADD_REMOTE_THREAD_RSP		= 0x0207,
	ORDER_CODE_DEL_REMOTE_THREAD_REQ		= 0x0208,
	ORDER_CODE_DEL_REMOTE_THREAD_RSP		= 0x0209,
	ORDER_CODE_RUN_THREAD_MSG_REQ			= 0x0210,
	ORDER_CODE_RUN_THREAD_MSG_RSP			= 0x0211,
	ORDER_CODE_TEST_RUN_THREAD_MSG_REQ		= 0x0212,
	ORDER_CODE_TEST_RUN_THREAD_MSG_RSP		= 0x0213,
	ORDER_CODE_LINK_UP_REQ					= 0x0214,
	ORDER_CODE_LINK_UP_RSP					= 0x0215,
	ORDER_CODE_LINK_DOWN_REQ				= 0x0216,
	ORDER_CODE_LINK_DOWN_RSP				= 0x0217,
	ORDER_CODE_HEARTBEAT_REQ				= 0x0218,
	ORDER_CODE_HEARTBEAT_RSP				= 0x0219,
	ORDER_CODE_RPCVER_REQ					= 0x0220,
	ORDER_CODE_RPCVER_RSP					= 0x0221,

	ORDER_CODE_END							= 0x8000,
} ORDER_CODE;

#define dave_byte_8_32(d, a0, a1, a2, a3) {u32 t; t=((((u32)(a0))<<24)&0xff000000); t+=((((u32)(a1))<<16)&0xff0000); t+=((((u32)(a2))<<8)&0xff00); t+=(((u32)(a3))&0xff); (d)=t;}
#define dave_byte_32_8(a0, a1, a2, a3, d) {u32 t; t=d; (a0)=(u8)((t)>>24); (a1)=(u8)((t)>>16); (a2)=(u8)((t)>>8); (a3)=(u8)(t);}
#define dave_byte_16(d, a0, a1) {u16 t; t=((((u16)(a0))<<8)&0xff00); t+=(((u16)(a1))&0xff); (d)=t;}
#define dave_byte_8(a0, a1, d) {u16 t; t=d; (a0)=(u8)((t)>>8); (a1)=(u8)(t);}

typedef void (*stack_receive_fun)(void *param, s32 socket, IPBaseInfo *pIPInfo, FRAMETYPE ver_type, ORDER_CODE order_id, ub frame_len, u8 *frame);

void base_rxtx_init(void);

void base_rxtx_exit(void);

dave_bool __base_rxtx_build__(SOCTYPE type, s32 socket, u16 port, s8 *file, ub line);
void __base_rxtx_clean__(s32 socket, s8 *file, ub line);
dave_bool base_rxtx_writes(s32 socket, ORDER_CODE order_id, MBUF *data);
dave_bool base_rxtx_send_ct(u8 dst_ip[4], u16 dst_port, s32 socket, ORDER_CODE order_id, MBUF *data);
dave_bool base_rxtx_send(u8 dst_ip[4], u16 dst_port, s32 socket, ORDER_CODE order_id, MBUF *data);
RetCode base_rxtx_input(SocketRead *pRead, stack_receive_fun result_fun, void *param);
RetCode base_rxtx_event(SocketRawEvent *pEvent, stack_receive_fun result_fun, void *param);

#define build_rxtx(type, socket, port) __base_rxtx_build__(type, socket, port, (s8 *)__func__, (ub)__LINE__)
#define clean_rxtx(socket) __base_rxtx_clean__(socket, (s8 *)__func__, (ub)__LINE__)
#define rxtx_writes base_rxtx_writes
#define rxtx_send_ct base_rxtx_send_ct
#define rxtx_send base_rxtx_send
#define rxtx_input base_rxtx_input
#define rxtx_event base_rxtx_event

#endif


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
	ORDER_CODE_LOG_RECORD_V2				= 0x0103,
	ORDER_CODE_LOG_CHAIN					= 0x0104,

	ORDER_CODE_RUN_INTERNAL_MSG_REQ 		= 0x0114,
	ORDER_CODE_RUN_INTERNAL_MSG_RSP 		= 0x0115,
	ORDER_CODE_RUN_INTERNAL_MSG_V2_REQ 		= 0x0116,
	ORDER_CODE_RUN_INTERNAL_MSG_V2_RSP 		= 0x0117,

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
	ORDER_CODE_SERVICE_STATEMENT			= 0x0222,

	/* system function order code */
	ORDER_CODE_SUPPORT_NO_CRC				= 0x7001,

	ORDER_CODE_END							= 0x8000,
} ORDER_CODE;

typedef void (*stack_receive_fun)(void *param, s32 socket, IPBaseInfo *pIPInfo, FRAMETYPE ver_type, ORDER_CODE order_id, ub frame_len, u8 *frame);

void base_rxtx_init(void);

void base_rxtx_exit(void);

dave_bool __base_rxtx_build__(SOCTYPE type, s32 socket, u16 port, s8 *file, ub line);
void __base_rxtx_clean__(s32 socket, s8 *file, ub line);
dave_bool base_rxtx_write(s32 socket, ORDER_CODE order_id, MBUF *data);
dave_bool base_rxtx_send_ct(u8 dst_ip[4], u16 dst_port, s32 socket, ORDER_CODE order_id, MBUF *data);
dave_bool base_rxtx_send(u8 dst_ip[4], u16 dst_port, s32 socket, ORDER_CODE order_id, MBUF *data);
RetCode base_rxtx_read(SocketRead *pRead, stack_receive_fun receive_fun, void *param);
RetCode base_rxtx_event(SocketRawEvent *pEvent, stack_receive_fun receive_fun, void *param);

#define build_rxtx(type, socket, port) __base_rxtx_build__(type, socket, port, (s8 *)__func__, (ub)__LINE__)
#define clean_rxtx(socket) __base_rxtx_clean__(socket, (s8 *)__func__, (ub)__LINE__)
#define rxtx_write base_rxtx_write
#define rxtx_send_ct base_rxtx_send_ct
#define rxtx_send base_rxtx_send
#define rxtx_read base_rxtx_read
#define rxtx_event base_rxtx_event

#endif


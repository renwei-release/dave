/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UIP_MSG_H__
#define __UIP_MSG_H__

#define UIP_THREAD_NAME "uip"

#define UIP_SERVER_HTTPs_PORT 1820
#define UIP_SERVER_H5_PORT 1821
#define UIP_SERVER_WeChat_PORT 1822

#define DAVE_UIP_METHOD_MAX_NUM (256)
#define DAVE_UIP_METHOD_MAX_LEN (64)

typedef enum {
	UIPType_uip,
	UIPType_json,
	UIPType_h5_form,
	UIPType_wechat_form,
} UIPType;

/* for UIP_REGISTER_REQ message */
typedef struct {
	s8 method[DAVE_UIP_METHOD_MAX_NUM][DAVE_UIP_METHOD_MAX_LEN];
	void *ptr;
} UIPRegisterReq;

/* for UIP_REGISTER_RSP message */
typedef struct {
	RetCode ret;
	s8 method[DAVE_UIP_METHOD_MAX_NUM][DAVE_UIP_METHOD_MAX_LEN];
	void *ptr;
} UIPRegisterRsp;

/* for UIP_UNREGISTER_REQ message */
typedef struct {
	s8 method[DAVE_UIP_METHOD_MAX_NUM][DAVE_UIP_METHOD_MAX_LEN];
	void *ptr;
} UIPUnregisterReq;

/* for UIP_UNREGISTER_RSP message */
typedef struct {
	RetCode ret;
	s8 method[DAVE_UIP_METHOD_MAX_NUM][DAVE_UIP_METHOD_MAX_LEN];
	void *ptr;
} UIPUnregisterRsp;

/* for UIP_DATA_RECV_REQ message */
typedef struct {
	s8 remote_address[DAVE_URL_LEN];
	ub remote_port;
	UIPType uip_type;
	s8 channel[DAVE_NORMAL_NAME_LEN];
	s8 method[DAVE_UIP_METHOD_MAX_LEN];
	MBUF *customer_body;
	MBUF *data;
	void *ptr;
} UIPDataRecvReq;

/* for UIP_DATA_RECV_RSP message */
typedef struct {
	RetCode ret;
	s8 method[DAVE_UIP_METHOD_MAX_LEN];
	MBUF *data;
	void *ptr;
} UIPDataRecvRsp;

/* for UIP_DATA_SEND_REQ message */
typedef struct {
	s8 remote_url[DAVE_URL_LEN];
	s8 channel[DAVE_NORMAL_NAME_LEN];
	s8 method[DAVE_UIP_METHOD_MAX_LEN];
	MBUF *customer_head;
	MBUF *customer_body;
	MBUF *data;
	void *ptr;
} UIPDataSendReq;

/* for UIP_DATA_SEND_RSP message */
typedef struct {
	RetCode ret;
	s8 method[DAVE_UIP_METHOD_MAX_LEN];
	MBUF *data;
	void *ptr;
} UIPDataSendRsp;

#endif


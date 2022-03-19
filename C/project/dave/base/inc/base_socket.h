/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_SOCKET_H__
#define __DAVE_SOCKET_H__

#define SOCKET_THREAD_NAME "socket"

#define INVALID_SOCKET_ID (-1)

/* for SOCKET_BIND_REQ message */
typedef struct {
	SocNetInfo NetInfo;
	void *ptr;
} SocketBindReq;

/* for SOCKET_BIND_RSP message */
typedef struct {
	s32 socket;
	SocNetInfo NetInfo;
	SOCKETINFO BindInfo;
	ThreadId thread_id;
	void *ptr;
} SocketBindRsp;

/* for SOCKET_CONNECT_REQ message */
typedef struct {
	SocNetInfo NetInfo;
	void *ptr;
} SocketConnectReq;

/* for SOCKET_CONNECT_RSP message */
typedef struct {
	s32 socket;
	SocNetInfo NetInfo;
	SOCKETINFO ConnectInfo;
	ThreadId thread_id;
	void *ptr;
} SocketConnectRsp;

/* for SOCKET_DISCONNECT_REQ message */
typedef struct {
	s32 socket;
	void *ptr;
} SocketDisconnectReq;

/* for SOCKET_DISCONNECT_RSP message */
typedef struct {
	s32 socket;
	SOCKETINFO result;
	void *ptr;
} SocketDisconnectRsp;

/* for SOCKET_PLUGIN message */
typedef struct {
	s32 father_socket;
	s32 child_socket;
	SocNetInfo NetInfo;
	ThreadId thread_id;
	void *ptr;
} SocketPlugIn;

/* for SOCKET_PLUGOUT message */
typedef struct {
	s32 socket;
	SOCKETINFO reason;
	SocNetInfo NetInfo;
	ThreadId thread_id;
	void *ptr;
} SocketPlugOut;

/* for SOCKET_READ message */
typedef struct {	
	s32 socket;
	IPBaseInfo IPInfo;
	ub data_len;
	MBUF *data;
	void *ptr;
} SocketRead;

/* for SOCKET_WRITE message */
typedef struct {
	s32 socket;
	IPBaseInfo IPInfo;
	ub data_len;
	MBUF *data;
	SOCKETINFO close_flag; 
} SocketWrite;

/* for SOCKET_NOTIFY message */
typedef struct {
	s32 socket;
	SOCKETINFO notify;
	ub data;
	void *ptr;
} SocketNotify;

/* for SOCKET_RAW_EVENT message */
typedef struct {
	s32 socket;
	s32 os_socket;
	SOCEVENT event;
	SocNetInfo NetInfo;
	MBUF *data;
	void *ptr;
} SocketRawEvent;

void base_socket_init(void);
void base_socket_exit(void);

#endif


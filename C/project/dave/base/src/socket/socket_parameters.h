/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SOCKET_PARAMETERS_H__
#define __SOCKET_PARAMETERS_H__

#define SOCKET_MAX (DAVE_SERVER_SUPPORT_SOCKET_MAX)
#define SOCKET_TCP_RECV_MAX_BUF (32768)
#define SOCKET_TCP_RECV_MIN_BUF (8192)
#define SOCKET_TCP_RECV_ADD_GRADIENT (2048)
#define SOCKET_TCP_RECV_DEC_GRADIENT (16384)
#define SOCKET_UDP_RECV_MAX_BUF (4096)

typedef enum {
	SOCKET_TYPE_SERVER_FATHER = 0,
	SOCKET_TYPE_SERVER_CHILD,
	SOCKET_TYPE_CLIENT,

	// Temp socket type.
	SOCKET_TYPE_CLIENT_WAIT,
	
	SOCKET_TYPE_MAX
} SOCKETTYPE;

#endif


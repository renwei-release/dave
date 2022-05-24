/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UIP_SERVER_HTTP_H__
#define __UIP_SERVER_HTTP_H__
#include "uip_parsing.h"

typedef RetCode (* http_server_recv_fun)(ThreadId src, HTTPRecvReq *pReq, void *pJson);

void uip_server_http_init(void);

void uip_server_http_exit(void);

dave_bool uip_server_http_start(u16 port, HTTPListenType type, s8 *path, http_server_recv_fun recv_fun);

void uip_server_http_stop(u16 port);

http_server_recv_fun uip_server_http_recv_fun(u16 port);

#endif


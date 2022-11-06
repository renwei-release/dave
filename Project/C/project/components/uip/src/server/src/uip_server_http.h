/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UIP_SERVER_HTTP_H__
#define __UIP_SERVER_HTTP_H__
#include "uip_parsing.h"
#include "uip_server_param.h"

void uip_server_http_init(void);

void uip_server_http_exit(void);

dave_bool uip_server_http_start(ub port, HTTPListenType type, s8 *path, uip_server_recv_fun recv_fun);

void uip_server_http_stop(ub port);

uip_server_recv_fun uip_server_http_recv_fun(ub port);

#endif


/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UIP_SERVER_DISTRIBUTOR_H__
#define __UIP_SERVER_DISTRIBUTOR_H__
#include "uip_parsing.h"
#include "uip_server_param.h"

void uip_server_distributor_init(void);

void uip_server_distributor_exit(void);

dave_bool uip_server_distributor_start(s8 *path_user, uip_server_recv_fun recv_fun);

void uip_server_distributor_stop(s8 *path_user);

uip_server_recv_fun uip_server_distributor_recv_fun(s8 *path);

#endif


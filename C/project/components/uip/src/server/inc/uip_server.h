/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UIP_SERVER_H__
#define __UIP_SERVER_H__
#include "dave_base.h"

void uip_server_init(MSGBODY *pMsg);

void uip_server_main(MSGBODY *pMsg);

void uip_server_exit(MSGBODY *pMsg);

void uip_server_restart(RESTARTREQMSG *pRestart);

ub uip_server_info(s8 *info_ptr, ub info_len);

#endif


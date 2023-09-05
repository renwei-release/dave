/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UIP_CLIENT_H__
#define __UIP_CLIENT_H__
#include "dave_base.h"

void uip_client_init(MSGBODY *pMsg);

void uip_client_main(MSGBODY *pMsg);

void uip_client_exit(MSGBODY *pMsg);

#endif


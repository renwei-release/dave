/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UIP_CLIENT_SEND_H__
#define __UIP_CLIENT_SEND_H__
#include "dave_base.h"
#include "uip_msg.h"

void uip_client_send(ThreadId src, UIPDataSendReq *pReq);

#endif


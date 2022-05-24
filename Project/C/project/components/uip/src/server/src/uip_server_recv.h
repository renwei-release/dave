/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UIP_SERVER_RECV_H__
#define __UIP_SERVER_RECV_H__
#include "dave_base.h"

RetCode uip_server_recv(UIPStack **ppRecvStack, ThreadId src, HTTPRecvReq *pReq, void *pJson);

#endif


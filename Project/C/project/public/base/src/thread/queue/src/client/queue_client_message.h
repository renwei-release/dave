/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __QUEUE_CLIENT_MESSAGE_H__
#define __QUEUE_CLIENT_MESSAGE_H__
#include "dave_base.h"

void queue_client_message_init(void);
void queue_client_message_exit(void);

void queue_client_message_update(QueueUpdateStateReq *pReq);

void queue_client_message_run_rsp(QueueRunMsgRsp *pRsp);

#endif


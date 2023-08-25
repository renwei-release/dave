/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __QUEUE_SERVER_MESSAGE_H__
#define __QUEUE_SERVER_MESSAGE_H__
#include "dave_base.h"

void queue_server_message_init(void);
void queue_server_message_exit(void);

void queue_server_message_upload(QueueUploadMsgReq *pReq);
void queue_server_message_download(ThreadId src, QueueDownloadMsgReq *pReq);
void queue_server_message_update_state_rsp(QueueUpdateStateRsp *pRsp);

ub queue_server_message_info(s8 *info_ptr, ub info_len);

#endif


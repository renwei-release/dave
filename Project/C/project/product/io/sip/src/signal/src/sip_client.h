/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SIP_CLIENT_H__
#define __SIP_CLIENT_H__

s32 sip_client_creat(s8 *server_ip, s8 *server_port, s8 *local_port);
void sip_client_release(s32 socket);

#endif


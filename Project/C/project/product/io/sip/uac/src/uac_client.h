/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UAC_CLIENT_H__
#define __UAC_CLIENT_H__

void uac_client_init(void);
void uac_client_exit(void);

s32 uac_client_creat(s8 *server_ip, s8 *server_port, s8 *local_port);
void uac_client_release(s32 socket);

#endif


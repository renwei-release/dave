/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __UAC_SERVER_H__
#define __UAC_SERVER_H__

void uac_server_init(void);
void uac_server_exit(void);

s32 uac_server_creat(s8 *port);
void uac_server_release(s32 socket);

#endif


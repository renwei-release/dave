/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __HTTP_RECV_DATA_H__
#define __HTTP_RECV_DATA_H__

void http_recv_data_init(void);
void http_recv_data_exit(void);
void http_recv_data_plugin(s32 socket, ub nginx_port);
void http_recv_data_plugout(s32 socket);

#endif


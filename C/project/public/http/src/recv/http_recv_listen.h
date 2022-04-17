/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __HTTP_RECV_LISTEN_H__
#define __HTTP_RECV_LISTEN_H__

void http_recv_listen_init(void);
void http_recv_listen_exit(void);
RetCode http_recv_listen_action(ThreadId src, ub port, HTTPListenType type, HTTPMathcRule rule, s8 *path);
RetCode http_recv_listen_close(ThreadId src, ub port);
void http_recv_listen_release(s32 socket);
s8 * http_recv_listen_thread(ub cgi_port);
ub http_recv_listen_port(ub cgi_port);

#endif


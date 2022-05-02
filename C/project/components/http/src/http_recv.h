/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __HTTP_RECV_H__
#define __HTTP_RECV_H__

void http_recv_init(void);
void http_recv_exit(void);
void http_recv_listen(ThreadId src, HTTPListenReq *pReq);
void http_recv_close(ThreadId src, HTTPCloseReq *pReq);

#endif


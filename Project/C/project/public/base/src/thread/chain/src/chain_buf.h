/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __CHAIN_BUF_H__
#define __CHAIN_BUF_H__

void chain_buf_init(void);

void chain_buf_exit(void);

void chain_buf_set(
	ThreadChain *pChain,
	ub msg_id, ub msg_len, void *msg_body);

MBUF * chain_buf_get(void);

#endif


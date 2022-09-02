/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_CLIENT_INTERNAL_BUFFER_H__
#define __SYNC_CLIENT_INTERNAL_BUFFER_H__

void sync_client_internal_buffer_init(void);

void sync_client_internal_buffer_exit(void);

void sync_client_internal_buffer_push(ub msg_id, ub msg_len, void *msg_body);

void sync_client_internal_buffer_pop(void);

#endif


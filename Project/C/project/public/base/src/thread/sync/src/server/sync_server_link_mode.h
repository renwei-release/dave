/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_SERVER_LINK_MODE_H__
#define __SYNC_SERVER_LINK_MODE_H__

void sync_server_link_mode_init(void);
void sync_server_link_mode_exit(void);

dave_bool sync_server_link_mode(s8 *ptr_a, s8 *ptr_b);

#endif


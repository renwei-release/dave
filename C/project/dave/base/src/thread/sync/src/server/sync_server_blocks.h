/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_SERVER_BLOCKS_H__
#define __SYNC_SERVER_BLOCKS_H__

void sync_server_blocks_command(ThreadId src, MsgBlocksReq *pReq);

ub sync_server_blocks_index_to_blocks_id(ub client_index);

#endif


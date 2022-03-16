/*
 * ================================================================================
 * (c) Copyright 2020 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.08.10.
 * ================================================================================
 */

#ifndef __SYNC_SERVER_BLOCKS_H__
#define __SYNC_SERVER_BLOCKS_H__

void sync_server_blocks_command(ThreadId src, MsgBlocksReq *pReq);

ub sync_server_blocks_index_to_blocks_id(ub client_index);

#endif


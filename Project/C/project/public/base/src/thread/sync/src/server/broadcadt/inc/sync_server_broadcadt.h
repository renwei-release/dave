/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_SERVER_BROADCADT_H__
#define __SYNC_SERVER_BROADCADT_H__

void sync_server_broadcadt_init(void);

void sync_server_broadcadt_exit(void);

RetCode sync_server_broadcadt(
	SyncClient *pSrcClient,
	ThreadId route_src, s8 *src,
	ThreadId route_dst, s8 *dst,
	ub msg_id,
	BaseMsgType msg_type,
	TaskAttribute src_attrib, TaskAttribute dst_attrib,
	u8 *msg_body, ub msg_len);

#endif


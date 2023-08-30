/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_CLIENT_QUEUE_H__
#define __SYNC_CLIENT_QUEUE_H__

BaseMsgType sync_client_queue_enable(SyncServer *pServer, s8 *src, s8 *dst, ub msg_id, BaseMsgType msg_type);

dave_bool sync_client_queue_upload(
	SyncServer *pServer,
	ThreadId route_src, ThreadId route_dst,
	s8 *src, s8 *dst,
	ub msg_id,
	MBUF *data);

void sync_client_queue_run(QueueRunMsgReq *pReq);

#endif


/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_BROADCAST_H__
#define __THREAD_BROADCAST_H__
#include "base_define.h"

dave_bool thread_broadcast_msg(
	ThreadStruct *thread_struct,
	BaseMsgType type,
	s8 *dst_name,
	ub msg_id,
	ub msg_len, u8 *msg_body,
	s8 *fun, ub line);

#endif


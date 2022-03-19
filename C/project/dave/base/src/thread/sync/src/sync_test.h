/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __SYNC_TEST_H__
#define __SYNC_TEST_H__

typedef ub (* sync_info_fun)(s8 *info, ub info_len);

void sync_test_req(ThreadId src, DebugReq *pReq, sync_info_fun info_fun);

void sync_test_rsp(MSGBODY *pMsg);

#endif


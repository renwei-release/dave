/*
 * ================================================================================
 * (c) Copyright 2018 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2018.05.07.
 * The synchronization service in the future may try to connect to all within
 * the framework of the server.
 * ================================================================================
 */

#ifndef __SYNC_TEST_H__
#define __SYNC_TEST_H__

typedef ub (* sync_info_fun)(s8 *info, ub info_len);

void sync_test_req(ThreadId src, DebugReq *pReq, sync_info_fun info_fun);

void sync_test_rsp(MSGBODY *pMsg);

#endif


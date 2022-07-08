/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifdef __DAVE_PRODUCT_TEST__
#include <iostream>
using namespace std;
#include "gtest.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "test_log.h"

// =====================================================================

TEST(base_case, BaseCase_RPC)
{
	RPCDebugReq *pReq = (RPCDebugReq *)thread_reset_msg(pReq);
	RPCDebugRsp *pRsp;

	pReq->s64_debug = 1234567890;

	pRsp = (RPCDebugRsp *)name_go("BASE", MSGID_RPC_DEBUG_REQ, pReq, MSGID_RPC_DEBUG_RSP);

	EXPECT_EQ(pReq->s64_debug, pRsp->s64_debug);
}

#endif


/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_3rdparty.h"
#if defined(GTEST_3RDPARTY)
#include <iostream>
using namespace std;
#include "gtest.h"
#include "dave_3rdparty.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "tools_log.h"

static void
_t_rpc_test_cases_1(void)
{
	MsgIdEchoReq req;
	MBUF *req_mbuf;
	void *pChainBson, *pRouterBson;
	MsgIdEchoRsp *rsp;
	ub msg_len;

	dave_strcpy(req.echo.thread, "1234567890", sizeof(req.echo.thread));

	req_mbuf = t_rpc_zip(NULL, NULL, MSGID_ECHO_REQ, &req, sizeof(req));

	t_rpc_unzip((s8 *)"test cases", &pChainBson, &pRouterBson, (void **)(&rsp), &msg_len, MSGID_ECHO_REQ, ms8(req_mbuf), mlen(req_mbuf));

	EXPECT_STREQ(rsp->echo.thread, "1234567890");

	thread_msg_release(rsp);
}

// =====================================================================

TEST(rpc_case, rpc_case_1) { _t_rpc_test_cases_1(); }

#endif


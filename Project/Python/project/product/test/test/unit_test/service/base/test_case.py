# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import unittest
from public import *


class BaseCase_RPC(unittest.TestCase):
    def test_rpc_1(self):
        pReq = RPCDebugReq()
        pReq.s64_debug = 1234567890
        pReq.mbuf_debug = None
        pReq.ptr = None

        pRsp = name_go('BASE', MSGID_RPC_DEBUG_REQ, pReq, MSGID_RPC_DEBUG_RSP, RPCDebugRsp)

        self.assertTrue(pReq.s64_debug==pRsp.s64_debug, "s64_debug")
        return


# =====================================================================


def test_case(suite, input_dir, output_dir):
    suite.addTest(unittest.makeSuite(BaseCase_RPC))
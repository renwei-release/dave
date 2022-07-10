package service

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/test/refittesting"
	"dave/public/auto"
	"dave/public/base"
	"unsafe"
)

// =====================================================================

func Base_case(t *refittesting.T) {
	req := auto.RPCDebugReq{}
	req.S16_debug = 12345
	req.Mbuf_debug = nil

	pRsp := (*auto.RPCDebugRsp)(base.Name_go("BASE", auto.MSGID_RPC_DEBUG_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req), auto.MSGID_RPC_DEBUG_RSP))
	if pRsp != nil {
		if pRsp.S16_debug == req.S16_debug {
			t.Log("RPCDebugReq success")
		} else {
			t.Error("RPCDebugReq failed")
		}
	} else {
		t.Error("get empty rsp!")
	}
}

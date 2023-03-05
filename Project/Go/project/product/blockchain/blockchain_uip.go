package blockchain

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/components/io"
	"dave/public/auto"
	"dave/public/base"
	"dave/public/tools"
	"unsafe"
)

const BLOCKCHAIN_UIP_METHOD = "blockchain"

type BlockChainReq struct {
	Option string `json:"option"`
	Param interface{} `json:"param"`
}

type BlockChainRsp struct {
	Option string `json:"option"`
	Param interface{} `json:"param"`
}

func _blockchain_recv_req(src_gid string, src_name string, src_id uint64, msg_len uint64, msg_body unsafe.Pointer) {
	pReq := (*auto.UIPDataRecvReq)(msg_body)

	method := tools.T_cgo_gobyte2gostring(pReq.Method[:])
	var req BlockChainReq
	base.T_mbuf2gojson(pReq.Data, &req)

	base.DAVEDEBUG("method:%v option:%v", method, req.Option)

	rsp_param, ret := blockchain_option(req.Option, req.Param)

	rsp := BlockChainRsp { 
        Option: req.Option, 
        Param: rsp_param,
	}

	io.Uip_recv_mbuf_rsp(src_id, ret, method, pReq.Ptr, base.T_gojson2mbuf(rsp))
}

// =====================================================================

func blockchain_uip_init() {
	io.Uip_register(BLOCKCHAIN_UIP_METHOD, _blockchain_recv_req)
}

func blockchain_uip_exit() {
	
}
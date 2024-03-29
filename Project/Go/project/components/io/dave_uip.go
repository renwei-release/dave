package io
/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"dave/public/auto"
	"dave/public/tools"
	"unsafe"
)

const UIP_THREAD_NAME = "uip"

func _register_rsp(src_gid string, src_name string, src_id uint64, msg_len uint64, msg_body unsafe.Pointer) {
	pRsp := (*auto.UIPRegisterRsp)(msg_body)

	method := tools.T_cgo_gobyte2gostring(pRsp.Method[:])

	base.DAVELOG("ret:%v method:%v", pRsp.Ret, method)
}

// =====================================================================

func Uip_register(method string, recv_req func(msg_src_gid string, src_name string, src_id uint64, msg_len uint64, msg_body unsafe.Pointer)) {
	pReq := auto.UIPRegisterReq{}

	copy(pReq.Method[:], method)

	base.Dave_system_function_table_add(auto.UIP_REGISTER_RSP, _register_rsp)
	base.Dave_system_function_table_add(auto.UIP_DATA_RECV_REQ, recv_req)

	base.Broadcast_msg(UIP_THREAD_NAME, auto.UIP_REGISTER_REQ, int(unsafe.Sizeof(pReq)), unsafe.Pointer(&pReq))
}

func Uip_recv_rsp(dst_id uint64, ret int64, method string, ptr uint64, data string) bool {
	pRsp := auto.UIPDataRecvRsp{}

	pRsp.Ret = ret
	copy(pRsp.Method[:], method)
	pRsp.Data = base.T_gostring2mbuf(data)
	pRsp.Ptr = ptr

	return base.Write_msg(dst_id, auto.UIP_DATA_RECV_RSP, int(unsafe.Sizeof(pRsp)), unsafe.Pointer(&pRsp))
}

func Uip_recv_mbuf_rsp(dst_id uint64, ret int64, method string, ptr uint64, data *auto.MBUF) bool {
	pRsp := auto.UIPDataRecvRsp{}

	pRsp.Ret = ret
	copy(pRsp.Method[:], method)
	pRsp.Data = data
	pRsp.Ptr = ptr

	return base.Write_msg(dst_id, auto.UIP_DATA_RECV_RSP, int(unsafe.Sizeof(pRsp)), unsafe.Pointer(&pRsp))
}
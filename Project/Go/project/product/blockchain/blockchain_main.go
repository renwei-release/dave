package blockchain

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/auto"
	"dave/public/base"
	"dave/public/tools"
	"unsafe"
)

func _fun_MSGID_DEBUG_RSP(src_id uint64, ptr uint64, debug_data_rsp string) {
	pRsp := auto.DebugRsp{}
	copy(pRsp.Msg[:], debug_data_rsp)
	pRsp.Ptr = ptr

	base.Write_msg(src_id, auto.MSGID_DEBUG_RSP, int(unsafe.Sizeof(pRsp)), unsafe.Pointer(&pRsp))
}

func _fun_MSGID_DEBUG_REQ(src_gid string, src_name string, src_id uint64, msg_len uint64, msg_body unsafe.Pointer) {
	pReq := (*auto.DebugReq)(msg_body)
	debug_data_req := tools.T_cgo_gobyte2gostring(pReq.Msg[:])

	debug_data_rsp := blockchain_debug(debug_data_req)

	_fun_MSGID_DEBUG_RSP(src_id, pReq.Ptr, debug_data_rsp)
}

func _main_msg_register() {
	base.Dave_system_function_table_add(auto.MSGID_DEBUG_REQ, _fun_MSGID_DEBUG_REQ)
}

func _main_msg_unregister() {
	base.Dave_system_function_table_del(auto.MSGID_DEBUG_REQ)
}

// =====================================================================

func Dave_product_init() {
	_main_msg_register()
}

func Dave_product_exit() {
	_main_msg_unregister()
}

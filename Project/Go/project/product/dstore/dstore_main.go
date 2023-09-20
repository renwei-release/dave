package dstore

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
	"dave/components/io"
	"dave/components/echo"
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

	var debug_data_rsp string
	debug_data_rsp = dstore_debug(debug_data_req)

	_fun_MSGID_DEBUG_RSP(src_id, pReq.Ptr, debug_data_rsp)
}

func _fun_MSGID_REMOTE_THREAD_ID_READY(src_gid string, src_name string, src_id uint64, msg_len uint64, msg_body unsafe.Pointer) {
	pReady := (*auto.ThreadRemoteIDReadyMsg)(msg_body)
	remote_thread_name := tools.T_cgo_gobyte2gostring(pReady.Remote_thread_name[:])

	if remote_thread_name == io.UIP_THREAD_NAME {
		dstore_uip_init()
	}
}

func _main_msg_register() {
	base.Dave_system_function_table_add(auto.MSGID_DEBUG_REQ, _fun_MSGID_DEBUG_REQ)
	base.Dave_system_function_table_add(auto.MSGID_REMOTE_THREAD_ID_READY, _fun_MSGID_REMOTE_THREAD_ID_READY)
	echo.Dave_echo_reg()
}

func _main_msg_unregister() {
	base.Dave_system_function_table_del(auto.MSGID_DEBUG_REQ)
	base.Dave_system_function_table_del(auto.MSGID_REMOTE_THREAD_ID_READY)
	echo.Dave_echo_unreg()
}

// =====================================================================

func Dave_product_init() {
	_main_msg_register()
}

func Dave_product_exit() {
	dstore_uip_exit()

	_main_msg_unregister()
}

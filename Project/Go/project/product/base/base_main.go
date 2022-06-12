package base

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

func _fun_MSGID_DEBUG_REQ(src_name string, src_id uint64, msg_len uint64, msg_body unsafe.Pointer) {
	pReq := (*auto.DebugReq)(msg_body)
	debug_data_req := tools.T_cgo_gobyte2gostring(pReq.Msg[:])

	var debug_data_rsp string
	debug_data_rsp = base_debug(debug_data_req)

	_fun_MSGID_DEBUG_RSP(src_id, pReq.Ptr, debug_data_rsp)
}

func _fun_RPC_DEBUG_REQ(remote_thread_name string) {
	req := auto.RPCDebugReq{}
	req.S16_debug = 12345
	req.Mbuf_debug = nil

	msg_body := base.Name_go(remote_thread_name, auto.MSGID_RPC_DEBUG_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req), auto.MSGID_RPC_DEBUG_RSP)
	if msg_body != nil {
		pRsp := (*auto.RPCDebugRsp)(msg_body)
		base.DAVELOG("name_go successfully! %d", pRsp.S16_debug)
	}
}

func _fun_MSGID_REMOTE_THREAD_ID_READY(src_name string, src_id uint64, msg_len uint64, msg_body unsafe.Pointer) {
	pReady := (*auto.ThreadRemoteIDReadyMsg)(msg_body)
	remote_thread_name := tools.T_cgo_gobyte2gostring(pReady.Remote_thread_name[:])
	remote_thread_id := pReady.Remote_thread_id
	base.DAVELOG("%s/%x", remote_thread_name, remote_thread_id)

	if remote_thread_name == "main_aib" {
		_fun_RPC_DEBUG_REQ(remote_thread_name)
	}
}

func _fun_MSGID_REMOTE_THREAD_ID_REMOVE(src_name string, src_id uint64, msg_len uint64, msg_body unsafe.Pointer) {
	pRemove := (*auto.ThreadRemoteIDRemoveMsg)(msg_body)
	remote_thread_name := tools.T_cgo_gobyte2gostring(pRemove.Remote_thread_name[:])
	remote_thread_id := pRemove.Remote_thread_id
	base.DAVELOG("%s/%x", remote_thread_name, remote_thread_id)
}

func _main_msg_register() {
	base.Dave_system_function_table_add(auto.MSGID_DEBUG_REQ, _fun_MSGID_DEBUG_REQ)
	base.Dave_system_function_table_add(auto.MSGID_REMOTE_THREAD_ID_READY, _fun_MSGID_REMOTE_THREAD_ID_READY)
	base.Dave_system_function_table_add(auto.MSGID_REMOTE_THREAD_ID_REMOVE, _fun_MSGID_REMOTE_THREAD_ID_REMOVE)
}

func _main_msg_unregister() {
	base.Dave_system_function_table_del(auto.MSGID_DEBUG_REQ)
	base.Dave_system_function_table_del(auto.MSGID_REMOTE_THREAD_ID_READY)
	base.Dave_system_function_table_del(auto.MSGID_REMOTE_THREAD_ID_REMOVE)
}

// =====================================================================

func Dave_product_init() {
	_main_msg_register()
}

func Dave_product_exit() {
	_main_msg_unregister()
}

package base
/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"dave/public/tools"
	"unsafe"
)

func _fun_MSGID_DEBUG_RSP(src_id uint64, ptr unsafe.Pointer, debug_data_rsp string) {
	pRsp := base.DebugRsp{}
	copy(pRsp.Msg[:], debug_data_rsp)
	pRsp.Ptr = ptr

	base.Write_msg(src_id, base.MSGID_DEBUG_RSP, int(unsafe.Sizeof(pRsp)), unsafe.Pointer(&pRsp))
}

func _fun_MSGID_DEBUG_REQ(src_name string, src_id uint64, msg_len uint64, msg_body unsafe.Pointer) {
	pReq := (*base.DebugReq)(msg_body)
	debug_data_req := tools.T_cgo_gobyte2gostring(pReq.Msg[:])

	var debug_data_rsp string
	debug_data_rsp = base_debug(debug_data_req)

	_fun_MSGID_DEBUG_RSP(src_id, pReq.Ptr, debug_data_rsp)
}

func _fun_MSGID_REMOTE_THREAD_ID_READY(src_name string, src_id uint64, msg_len uint64, msg_body unsafe.Pointer) {
	pReady := (*base.ThreadRemoteIDReadyMsg)(msg_body)
	remote_thread_name := tools.T_cgo_gobyte2gostring(pReady.Remote_thread_name[:])
	remote_thread_id := pReady.Remote_thread_id
	base.DAVELOG("%s/%x", remote_thread_name, remote_thread_id)
}

func _fun_MSGID_REMOTE_THREAD_ID_REMOVE(src_name string, src_id uint64, msg_len uint64, msg_body unsafe.Pointer) {
	pRemove := (*base.ThreadRemoteIDRemoveMsg)(msg_body)
	remote_thread_name := tools.T_cgo_gobyte2gostring(pRemove.Remote_thread_name[:])
	remote_thread_id := pRemove.Remote_thread_id
	base.DAVELOG("%s/%x", remote_thread_name, remote_thread_id)
}

func _main_msg_register() {
	base.Dave_system_function_table_add(base.MSGID_DEBUG_REQ, _fun_MSGID_DEBUG_REQ)
	base.Dave_system_function_table_add(base.MSGID_REMOTE_THREAD_ID_READY, _fun_MSGID_REMOTE_THREAD_ID_READY)
	base.Dave_system_function_table_add(base.MSGID_REMOTE_THREAD_ID_REMOVE, _fun_MSGID_REMOTE_THREAD_ID_REMOVE)
}

func _main_msg_unregister() {
	base.Dave_system_function_table_del(base.MSGID_DEBUG_REQ)
	base.Dave_system_function_table_del(base.MSGID_REMOTE_THREAD_ID_READY)
	base.Dave_system_function_table_del(base.MSGID_REMOTE_THREAD_ID_REMOVE)
}

// =====================================================================

func Dave_product_init() {
	_main_msg_register()
}

func Dave_product_exit() {
	_main_msg_unregister()
}
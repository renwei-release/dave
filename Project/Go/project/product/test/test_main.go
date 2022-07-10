package test

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/test/test/test_service"
	"dave/public/auto"
	"dave/public/base"
	"dave/public/tools"
	"time"
	"unsafe"
)

func _test_service_test_exit() {
	base.Dave_poweroff()
}

var _test_timer = time.AfterFunc(time.Second*360, _test_service_test_exit)

func _test_stop_timer() {
	_test_timer.Stop()
}

func _test_start_timer(second time.Duration) {
	_test_stop_timer()
	_test_timer = time.AfterFunc(time.Second*second, _test_service_test_exit)
}

func _fun_MSGID_REMOTE_THREAD_ID_READY(src_name string, src_id uint64, msg_len uint64, msg_body unsafe.Pointer) {
	pReady := (*auto.ThreadRemoteIDReadyMsg)(msg_body)
	globally_identifier := tools.T_cgo_gobyte2gostring(pReady.Globally_identifier[:])
	remote_thread_name := tools.T_cgo_gobyte2gostring(pReady.Remote_thread_name[:])
	remote_thread_id := pReady.Remote_thread_id

	_test_stop_timer()

	test_service.Test_service(globally_identifier, remote_thread_name, remote_thread_id)

	_test_start_timer(3)
}

func _main_msg_register() {
	base.Dave_system_function_table_add(auto.MSGID_REMOTE_THREAD_ID_READY, _fun_MSGID_REMOTE_THREAD_ID_READY)
}

func _main_msg_unregister() {
	base.Dave_system_function_table_del(auto.MSGID_REMOTE_THREAD_ID_READY)
}

// =====================================================================

func Dave_product_init() {
	_main_msg_register()

	_test_start_timer(360)
}

func Dave_product_exit() {
	_main_msg_unregister()
}

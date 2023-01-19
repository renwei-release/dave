//go:build windows
// +build windows

package base

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

/*
#cgo CFLAGS: -I./inc -I./inc/define
#cgo LDFLAGS: -L./lib -lcygwinBASE -lntdll -lkernel32 -lws2_32 -lcygwin1 -liphlpapi -lrpcrt4 -lIPHLPAPI -lcyggcc_s-seh-1
#include <dave_base.h>
#include <stdio.h>
#include <stdlib.h>

void _go_init(void *);
void _go_main(void *);
void _go_exit(void *);
int _go_self_check_callback(int);
*/
import "C"
import (
	"unsafe"
)

type DllMsgBody struct {
	msg_src_gid [64]byte
	msg_src_name [128]byte
	msg_src      uint64
	msg_dst_name [128]byte
	msg_dst      uint64
	msg_id       uint64
	msg_len      uint64
	msg_check    uint64
	msg_body     unsafe.Pointer
}

var pre_init = Dave_go_system_pre_init()

var _product_init_fun func()
var _product_exit_fun func()

func _reset_verno() {
	c_my_verno := C.CString(Dave_verno())
	C.dave_dll_reset_verno(c_my_verno)
	C.free(unsafe.Pointer(c_my_verno))
}

//export _go_init
func _go_init(c_data unsafe.Pointer) {
	_product_init_fun()
}

//export _go_main
func _go_main(c_data unsafe.Pointer) {
	Dave_msg_process((*DllMsgBody)(c_data))
}

//export _go_exit
func _go_exit(c_data unsafe.Pointer) {
	_product_exit_fun()
}

//export _go_self_check_callback
func _go_self_check_callback(callback_input C.int) C.int {
	return callback_input
}

func _go_self_check() C.int {
	check_str := C.CString("123456")

	ret := C.dave_dll_self_check(check_str, C.int(123456), C.float(123456.123456), C.dll_checkback_fun(C._go_self_check_callback))

	C.free(unsafe.Pointer(check_str))
	return ret
}

// =====================================================================

func Dave_go_system_pre_init() bool {
	/*
	 * Preventing the system from being called in advance
	 * without initialization call
	 */
	 _reset_verno()

	 return true
}

func Dave_go_init(product_verno string, work_mode string, sync_domain string, init_fun func(), exit_fun func()) {
	_product_init_fun = init_fun
	_product_exit_fun = exit_fun

	c_my_verno := C.CString(Dave_verno())
	if product_verno != "" {
		c_my_verno = C.CString(product_verno)
	}
	c_work_mode := C.CString(work_mode)
	c_sync_domain := C.CString(sync_domain)
	thread_number := 0

	C.dave_dll_init(
		c_my_verno, c_work_mode,
		C.int(thread_number),
		C.dll_callback_fun(C._go_init), C.dll_callback_fun(C._go_main), C.dll_callback_fun(C._go_exit),
		c_sync_domain)

	C.free(unsafe.Pointer(c_my_verno))
	C.free(unsafe.Pointer(c_work_mode))
	C.free(unsafe.Pointer(c_sync_domain))
}

func Dave_go_running() {
	ret := _go_self_check()
	if ret == 0 {
		C.dave_dll_running()
	}
}

func Dave_go_exit() {
	C.dave_dll_exit()
}

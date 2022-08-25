//go:build linux
// +build linux

package base

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

/*
#cgo CFLAGS: -I./inc -I./inc/define
#cgo LDFLAGS: -Wl,-rpath,./lib
#cgo LDFLAGS: -L./lib -llinuxBASE -ljemalloc -lpthread -lrt -ldl -lm -lstdc++ -lutil
#include <dave_base.h>
#include <stdio.h>
#include <stdlib.h>

int dave_go_self_check_callback(int callback_input);
void dave_go_init(void *);
void dave_go_main(void *);
void dave_go_exit(void *);
*/
import "C"
import (
	"unsafe"
)

type DllMsgBody struct {
	msg_src_name [128]byte
	msg_src      uint64
	msg_dst_name [128]byte
	msg_dst      uint64
	msg_id       uint64
	msg_len      uint64
	msg_check    uint64
	msg_body     unsafe.Pointer
}

var _product_init_fun func()
var _product_exit_fun func()

//export dave_go_init
func dave_go_init(c_data unsafe.Pointer) {
	_product_init_fun()

	dave_module_init()
}

//export dave_go_main
func dave_go_main(c_data unsafe.Pointer) {
	Dave_msg_process((*DllMsgBody)(c_data))
}

//export dave_go_exit
func dave_go_exit(c_data unsafe.Pointer) {
	dave_module_exit()

	_product_exit_fun()
}

//export dave_go_self_check_callback
func dave_go_self_check_callback(callback_input C.int) C.int {
	return callback_input
}

func _dave_go_self_check() C.int {
	check_str := C.CString("123456")

	ret := C.dave_dll_self_check(check_str, C.int(123456), C.float(123456.123456), C.dll_checkback_fun(C.dave_go_self_check_callback))

	C.free(unsafe.Pointer(check_str))
	return ret
}

// =====================================================================

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
		C.dll_callback_fun(C.dave_go_init), C.dll_callback_fun(C.dave_go_main), C.dll_callback_fun(C.dave_go_exit),
		c_sync_domain)

	C.free(unsafe.Pointer(c_my_verno))
	C.free(unsafe.Pointer(c_work_mode))
	C.free(unsafe.Pointer(c_sync_domain))
}

func Dave_go_running() {
	ret := _dave_go_self_check()
	if ret == 0 {
		C.dave_dll_running()
	}
}

func Dave_go_exit() {
	C.dave_dll_exit()
}

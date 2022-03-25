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
#cgo LDFLAGS: -lpthread -lrt -ldl -lm -lgomp -lz -lstdc++ -lutil -L./lib -ljemalloc -llinuxBASE
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
	msg_src_name [128] byte
	msg_src uint64
	msg_dst_name [128] byte
	msg_dst uint64
	msg_id uint64
	msg_len uint64
	msg_check uint64
	msg_body unsafe.Pointer
}

var _product_init_fun func()
var _product_exit_fun func()

//export dave_go_self_check_callback
func dave_go_self_check_callback(callback_input C.int) C.int {
	return callback_input
}

//export dave_go_init
func dave_go_init(c_data unsafe.Pointer) {
	_product_init_fun()
}

//export dave_go_main
func dave_go_main(c_data unsafe.Pointer) {
	Dave_msg_process((* DllMsgBody)(c_data))
}

//export dave_go_exit
func dave_go_exit(c_data unsafe.Pointer) {
	_product_exit_fun()
}

func dave_go_self_check() C.int {
	check_str := C.CString("123456")

	ret := C.dave_dll_self_check(check_str, C.int(123456), C.float(123456.123456), C.dll_checkback_fun(C.dave_go_self_check_callback))

	C.free(unsafe.Pointer(check_str))
	return ret
}

// =====================================================================

func Dave_go_init(init_fun func(), exit_fun func()) {
	_product_init_fun = init_fun
	_product_exit_fun = exit_fun

	my_verno := C.CString(Dave_verno())
	model := C.CString("Outer Loop")

	C.dave_dll_init(my_verno, model, C.dll_callback_fun(C.dave_go_init), C.dll_callback_fun(C.dave_go_main), C.dll_callback_fun(C.dave_go_exit))

	C.free(unsafe.Pointer(my_verno))
	C.free(unsafe.Pointer(model))
}

func Dave_go_running() {
	ret := dave_go_self_check()
	if ret == 0 {
		C.dave_dll_running()
	}
	C.dave_dll_wait_dll_exit()
}

func Dave_go_exit() {
	C.dave_dll_exit()
}
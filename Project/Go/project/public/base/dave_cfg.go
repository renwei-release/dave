package base
/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

 /*
#include <dave_base.h>
#include <stdio.h>
#include <stdlib.h>
*/
import "C"
import (
	"unsafe"
)

// =====================================================================

func Cfg_set(cfg_name string, cfg_ptr string) bool {
	c_cfg_name := C.CString(cfg_name)
	c_cfg_ptr := C.CString(cfg_ptr)

	ret := C.dave_dll_cfg_set(c_cfg_name, c_cfg_ptr)

	C.free(unsafe.Pointer(c_cfg_name))
	C.free(unsafe.Pointer(c_cfg_ptr))

	if ret == 0 {
		return true
	}

	return false
}

func Cfg_get(cfg_name string) string {
	var go_byte [4096]byte

	c_cfg_name := C.CString(cfg_name)
	c_cfg_ptr := C.CString(string(go_byte[:]))

	go_string := ""

	if C.dave_dll_cfg_get(c_cfg_name, c_cfg_ptr, C.int(len(go_byte))) >= 0 {
		go_string = C.GoString(c_cfg_ptr)
	}

	C.free(unsafe.Pointer(c_cfg_name))
	C.free(unsafe.Pointer(c_cfg_ptr))

	return go_string
}

func Cfg_remote_set(cfg_name string, cfg_ptr string) bool {
	c_cfg_name := C.CString(cfg_name)
	c_cfg_ptr := C.CString(cfg_ptr)

	ret := C.dave_dll_cfg_remote_set(c_cfg_name, c_cfg_ptr)

	C.free(unsafe.Pointer(c_cfg_name))
	C.free(unsafe.Pointer(c_cfg_ptr))

	if ret == 0 {
		return true
	}

	return false
}

func Cfg_remote_get(cfg_name string) string {
	var go_byte [4096]byte

	c_cfg_name := C.CString(cfg_name)
	c_cfg_ptr := C.CString(string(go_byte[:]))

	go_string := ""

	if C.dave_dll_cfg_remote_get(c_cfg_name, c_cfg_ptr, C.int(len(go_byte))) >= 0 {
		go_string = C.GoString(c_cfg_ptr)
	}

	C.free(unsafe.Pointer(c_cfg_name))
	C.free(unsafe.Pointer(c_cfg_ptr))

	return go_string
}
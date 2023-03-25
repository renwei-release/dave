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
	"fmt"
	"strconv"
	"unsafe"
)

// =====================================================================

func Cfg_set(cfg_name string, cfg_value string) bool {
	Dave_go_system_pre_init()

	c_cfg_name := C.CString(cfg_name)
	c_cfg_value := C.CString(cfg_value)

	ret := C.dave_dll_cfg_set(c_cfg_name, c_cfg_value)

	C.free(unsafe.Pointer(c_cfg_name))
	C.free(unsafe.Pointer(c_cfg_value))

	if ret == 0 {
		return true
	}

	return false
}

func Cfg_get(cfg_name string, default_value string) string {
	Dave_go_system_pre_init()

	var go_byte [4096]byte

	c_cfg_name := C.CString(cfg_name)
	c_cfg_value := C.CString(string(go_byte[:]))

	go_string := ""

	if C.dave_dll_cfg_get(c_cfg_name, c_cfg_value, C.int(len(go_byte))) >= 0 {
		go_string = C.GoString(c_cfg_value)
	} else {
		if len(default_value) > 0 { 
			Cfg_set(cfg_name, default_value)
		}
		go_string = default_value
	}

	C.free(unsafe.Pointer(c_cfg_name))
	C.free(unsafe.Pointer(c_cfg_value))

	return go_string
}

func Cfg_set_ub(cfg_name string, cfg_value int64) bool {
	str_value := strconv.FormatInt(cfg_value, 10)

	return Cfg_set(cfg_name, str_value)
}

func Cfg_get_ub(cfg_name string, default_value int64) int64 {
	default_str_value := strconv.FormatInt(default_value, 10)

	str_value := Cfg_get(cfg_name, default_str_value)

	int64_value, err := strconv.ParseInt(str_value, 10, 64)
	if err != nil {
		fmt.Printf("default_str_value:%v default_value:%v str_value:%v err:%v\n",
			default_str_value, default_value, str_value, err)
		return 0
	}
	return int64_value
}

func Rcfg_set(cfg_name string, cfg_value string, ttl int) bool {
	c_cfg_name := C.CString(cfg_name)
	c_cfg_value := C.CString(cfg_value)

	ret := C.dave_dll_cfg_remote_set(c_cfg_name, c_cfg_value, C.int(ttl))

	C.free(unsafe.Pointer(c_cfg_name))
	C.free(unsafe.Pointer(c_cfg_value))

	if ret == 0 {
		return true
	}

	return false
}

func Rcfg_get(cfg_name string) string {
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
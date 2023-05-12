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

void _go_cfg_reg_fun(void *name_ptr, int name_len, void *value_ptr, int value_len);
*/
import "C"
import (
	"fmt"
	"strconv"
	"unsafe"
	"sync"
	"dave/public/tools"
)

type cfg_update_fun func(cfg_name string, cfg_value string)

var (
    cfg_update_fun_table = map[string]cfg_update_fun{}
    rwCfgMutex  = sync.RWMutex{}
)

func _cfg_update_fun_add(cfg_name string, update_fun cfg_update_fun) {
    rwCfgMutex.Lock()
    defer rwCfgMutex.Unlock()

    cfg_update_fun_table[cfg_name] = update_fun
}

func _cfg_update_fun_inq(cfg_name string) (cfg_update_fun, bool) {
    rwCfgMutex.RLock()
    defer rwCfgMutex.RUnlock()

    fun, exists := cfg_update_fun_table[cfg_name]
    return fun, exists
}

//export _go_cfg_reg_fun
func _go_cfg_reg_fun(cfg_name_ptr unsafe.Pointer, cfg_name_len C.int, cfg_value_ptr unsafe.Pointer, cfg_value_len C.int) {
	cfg_name := tools.T_cgo_cbin2gostring(int64(cfg_name_len), cfg_name_ptr)
	cfg_value := tools.T_cgo_cbin2gostring(int64(cfg_value_len), cfg_value_ptr)

	DAVEDEBUG("%s:%s", cfg_name, cfg_value)

	if fun, exists := _cfg_update_fun_inq(cfg_name); exists {
		fun(cfg_name, cfg_value)
	}
}

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

func Cfg_del(cfg_name string) bool {
	Dave_go_system_pre_init()

	c_cfg_name := C.CString(cfg_name)

	ret := C.dave_dll_cfg_del(c_cfg_name)

	C.free(unsafe.Pointer(c_cfg_name))

	if ret == 0 {
		return true
	}

	return false
}

func Cfg_reg(cfg_name string, reg_fun func(name string, value string)) bool {
	Dave_go_system_pre_init()

	c_cfg_name := C.CString(cfg_name)

	ret := C.dave_dll_cfg_reg(c_cfg_name, C.dll_cfg_reg_fun(C._go_cfg_reg_fun))

	C.free(unsafe.Pointer(c_cfg_name))

	if ret == 0 {
		_cfg_update_fun_add(cfg_name, reg_fun)
		return true
	}

	return false
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
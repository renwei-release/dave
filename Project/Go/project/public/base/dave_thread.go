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
	"dave/public/tools"
	"fmt"
	"reflect"
	"runtime"
	"strings"
	"unsafe"
)

// =====================================================================

func Thread_msg(msg_len int) unsafe.Pointer {
	pc, _, __LINE__, _ := runtime.Caller(2)
	funcNamearray := strings.Split(runtime.FuncForPC(pc).Name(), ".")
	__func__ := funcNamearray[len(funcNamearray)-1]

	c_func := (*C.char)(tools.T_cgo_gostring2cstring(__func__))

	msg_ptr := C.dave_dll_thread_msg(C.int(msg_len), c_func, C.int(__LINE__))

	return msg_ptr
}

func Write_msg(dst interface{}, msg_id int, msg_len int, msg_ptr unsafe.Pointer) bool {
	pc, _, __LINE__, _ := runtime.Caller(2)
	funcNamearray := strings.Split(runtime.FuncForPC(pc).Name(), ".")
	__func__ := funcNamearray[len(funcNamearray)-1]

	c_func := (*C.char)(tools.T_cgo_gostring2cstring(__func__))

	ret := C.int(-1)

	switch dst.(type) {
	case uint64:
		ret = C.dave_dll_thread_id_msg(C.ulonglong(dst.(uint64)), C.int(msg_id), C.int(msg_len), msg_ptr, c_func, C.int(__LINE__))
	case string:
		c_thread_name := C.CString(dst.(string))
		ret = C.dave_dll_thread_name_msg(c_thread_name, C.int(msg_id), C.int(msg_len), msg_ptr, c_func, C.int(__LINE__))
		C.free(unsafe.Pointer(c_thread_name))
	default:
		fmt.Printf("Write_msg invalid type:%s\n", reflect.TypeOf(dst).Name())
	}

	if ret < 0 {
		DAVELOG("%x write msg %d failed! <%s:%d>", dst, msg_id, __func__, __LINE__)
		return false
	}
	return true
}

func Sync_msg(dst string, req_id int, req_len int, req_ptr unsafe.Pointer, rsp_id int, rsp_len int, rsp_ptr unsafe.Pointer) bool {
	pc, _, __LINE__, _ := runtime.Caller(2)
	funcNamearray := strings.Split(runtime.FuncForPC(pc).Name(), ".")
	__func__ := funcNamearray[len(funcNamearray)-1]

	c_func := (*C.char)(tools.T_cgo_gostring2cstring(__func__))

	c_dst := C.CString(dst)
	ret := C.dave_dll_thread_sync_msg(c_dst, C.int(req_id), C.int(req_len), req_ptr, C.int(rsp_id), C.int(rsp_len), rsp_ptr, c_func, C.int(__LINE__))
	C.free(unsafe.Pointer(c_dst))

	if ret == nil {
		return false
	}

	return true
}

func Name_go(dst string, req_id int, req_len int, req_ptr unsafe.Pointer, rsp_id int) unsafe.Pointer {
	pc, _, __LINE__, _ := runtime.Caller(2)
	funcNamearray := strings.Split(runtime.FuncForPC(pc).Name(), ".")
	__func__ := funcNamearray[len(funcNamearray)-1]

	c_func := (*C.char)(tools.T_cgo_gostring2cstring(__func__))

	c_dst := C.CString(dst)
	ret := C.dave_dll_thread_sync_msg(c_dst, C.int(req_id), C.int(req_len), req_ptr, C.int(rsp_id), C.int(0), nil, c_func, C.int(__LINE__))
	C.free(unsafe.Pointer(c_dst))

	return ret
}

func Broadcast_msg(dst string, msg_id int, msg_len int, msg_ptr unsafe.Pointer) bool {
	pc, _, __LINE__, _ := runtime.Caller(2)
	funcNamearray := strings.Split(runtime.FuncForPC(pc).Name(), ".")
	__func__ := funcNamearray[len(funcNamearray)-1]

	c_func := (*C.char)(tools.T_cgo_gostring2cstring(__func__))
	c_dst := C.CString(dst)

	ret := C.dave_dll_thread_broadcast_msg(c_dst, C.int(msg_id), C.int(msg_len), msg_ptr, c_func, C.int(__LINE__))

	C.free(unsafe.Pointer(c_dst))

	if ret < 0 {
		DAVELOG("%s broadcast msg %d failed! <%s:%d>", dst, msg_id, __func__, __LINE__)
		return false
	}
	return true
}

func Gid_msg(gid string, thread_name string, msg_id int, msg_len int, msg_ptr unsafe.Pointer) bool {
	pc, _, __LINE__, _ := runtime.Caller(2)
	funcNamearray := strings.Split(runtime.FuncForPC(pc).Name(), ".")
	__func__ := funcNamearray[len(funcNamearray)-1]

	c_func := (*C.char)(tools.T_cgo_gostring2cstring(__func__))

	ret := C.int(-1)
	c_gid_name := C.CString(gid)
	c_thread_name := C.CString(thread_name)
	ret = C.dave_dll_thread_gid_msg(c_gid_name, c_thread_name, C.int(msg_id), C.int(msg_len), msg_ptr, c_func, C.int(__LINE__))
	C.free(unsafe.Pointer(c_gid_name))
	C.free(unsafe.Pointer(c_thread_name))

	if ret < 0 {
		DAVELOG("%s, %s write msg %d failed! <%s:%d>", gid, thread_name, msg_id, __func__, __LINE__)
		return false
	}
	return true
}

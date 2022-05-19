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
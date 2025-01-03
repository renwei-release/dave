package base

/*
 * Copyright (c) 2023 Renwei
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
	"runtime"
	"strings"
	"unsafe"
)

func _dave_go_log(log_string string, log_type int) {
	pc, _, __LINE__, _ := runtime.Caller(2)
	funcNamearray := strings.Split(runtime.FuncForPC(pc).Name(), ".")
	__func__ := funcNamearray[len(funcNamearray)-1]

	c_func := C.CString(__func__)
	c_log_string := C.CString(log_string)

	C.dave_dll_log(c_func, C.int(__LINE__), c_log_string, C.int(log_type))

	C.free(unsafe.Pointer(c_func))
	C.free(unsafe.Pointer(c_log_string))
}

// =====================================================================

func DAVEDEBUG(format string, a ...interface{}) {
	//log_string := fmt.Sprintf(format, a...)
	//_dave_go_log(log_string, 0)
}

func DAVETRACE(format string, a ...interface{}) {
	log_string := fmt.Sprintf(format, a...)
	_dave_go_log(log_string, 1)
}

func DAVELOG(format string, a ...interface{}) {
	log_string := fmt.Sprintf(format, a...)
	_dave_go_log(log_string, 2)
}

func DAVEABNORMAL(format string, a ...interface{}) {
	log_string := fmt.Sprintf(format, a...)
	_dave_go_log(log_string, 3)
}
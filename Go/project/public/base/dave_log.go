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
	"runtime"
	"strings"
	"dave/public/tools"
)

func _dave_go_log(log_string string) {
	pc, _, __LINE__, _ := runtime.Caller(2)
	funcNamearray := strings.Split(runtime.FuncForPC(pc).Name(), ".")
	__func__ := funcNamearray[len(funcNamearray)-1]

    c_func := (*C.char)(tools.T_cgo_gostring2cstring(__func__))
    c_log_string := (*C.char)(tools.T_cgo_gostring2cstring(log_string))
    C.dave_dll_log(c_func, C.int(__LINE__), c_log_string)
}

// =====================================================================

func DAVELOG(format string, a ...interface{}) {
	log_string := fmt.Sprintf(format, a...)
	_dave_go_log(log_string)
}

func DAVEDEBUG(format string, a ...interface{}) {
	/*
	log_string := fmt.Sprintf(format, a...)
	_dave_go_log(log_string)
	*/
}
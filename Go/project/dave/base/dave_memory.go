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
	"runtime"
	"strings"
	"unsafe"
	"dave/dave/tools"
)

// =====================================================================

func Dave_mmalloc(data_len int) * MBUF{
	pc, _, __LINE__, _ := runtime.Caller(2)
	funcNamearray := strings.Split(runtime.FuncForPC(pc).Name(), ".")
	__func__ := funcNamearray[len(funcNamearray)-1]

	c_func := (*C.char)(tools.T_cgo_gostring2cstring(__func__))

	mbuf_ptr := C.dave_dll_mmalloc(C.int(data_len), c_func, C.int(__LINE__))

	return (* MBUF)(mbuf_ptr)
}

func Dave_mfree(ptr * MBUF) {
	pc, _, __LINE__, _ := runtime.Caller(2)
	funcNamearray := strings.Split(runtime.FuncForPC(pc).Name(), ".")
	__func__ := funcNamearray[len(funcNamearray)-1]

	c_func := (*C.char)(tools.T_cgo_gostring2cstring(__func__))

	C.dave_dll_mfree(unsafe.Pointer(ptr), c_func, C.int(__LINE__))
}
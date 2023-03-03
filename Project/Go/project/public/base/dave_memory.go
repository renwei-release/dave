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
	"dave/public/auto"
	"unsafe"
)

// =====================================================================

func Dave_mmalloc(data_len int) *auto.MBUF {
	mbuf_ptr := C.dave_dll_mmalloc(C.int(data_len), nil, C.int(0))
	return (*auto.MBUF)(mbuf_ptr)
}

func Dave_mfree(ptr *auto.MBUF) {
	if ptr == nil {
		return
	}
	C.dave_dll_mfree(unsafe.Pointer(ptr), nil, C.int(0))
}

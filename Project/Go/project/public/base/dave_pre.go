//go:build !__DAVE_PRODUCT_API__
/*
 * API products do not need pre-initialization
 */

package base

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import "C"
import (
	"unsafe"
)

func _reset_verno() {
	c_my_verno := C.CString(Dave_verno())
	C.dave_dll_reset_verno(c_my_verno)
	C.free(unsafe.Pointer(c_my_verno))
}

// =====================================================================

func Dave_go_system_pre_init() bool {
	/*
	 * Preventing the system from being called in advance
	 * without initialization call
	 */
	 _reset_verno()

	 return true
}

var pre_init = Dave_go_system_pre_init()
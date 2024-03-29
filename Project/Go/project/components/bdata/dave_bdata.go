package bdata

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"dave/public/auto"
	"fmt"
	"unsafe"
)

const BDATA_THREAD_NAME = "bdata"

// =====================================================================

func BDATALOG(sub string, format string, log ...interface{}) {
	req := auto.BDataLogReq{}
 
	log_string := fmt.Sprintf(format, log...)
 
	copy(req.Version[:], base.Dave_verno())
	copy(req.Sub_flag[:], sub)

	req.Log_data = base.T_gostring2mbuf(log_string)
	req.Ptr = 0
 
	base.Write_msg(BDATA_THREAD_NAME, auto.BDATA_LOG_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req))
}
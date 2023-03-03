package store

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

const STORE_THREAD_NAME = "store"

// =====================================================================

func STORESQL(format string, sql ...interface{}) {
	sql_string := fmt.Sprintf(format, sql...)

	req := auto.StoreMysqlReq{}
	req.Sql = base.T_gostring2mbuf(sql_string)
	req.Ptr = 0

	pRsp := (*auto.StoreMysqlRsp)(base.Name_co(STORE_THREAD_NAME, auto.STORE_MYSQL_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req), auto.STORE_MYSQL_RSP))
	if pRsp == nil {
		return
	}

	if pRsp.Ret != auto.RetCode_OK && pRsp.Ret != auto.RetCode_empty_data {
		base.DAVELOG("ret:%s on sql:%s", auto.T_auto_RetCode_str(pRsp.Ret), sql_string)
	}

	base.Dave_mfree(pRsp.Data)
}
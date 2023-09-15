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
	"dave/public/tools"
	"fmt"
	"unsafe"
	"errors"
	"strconv"
)

const STORE_THREAD_NAME = "store"

func _store_sql_co(sql_string string) (*auto.StoreMysqlRsp, error){
	req := auto.StoreMysqlReq{}
	req.Sql = base.T_gostring2mbuf(sql_string)
	req.Ptr = 0

	pRsp := (*auto.StoreMysqlRsp)(base.Write_co(STORE_THREAD_NAME, auto.STORE_MYSQL_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req), auto.STORE_MYSQL_RSP))
	if pRsp == nil {
		return nil, errors.New("co timer out")
	}

	return pRsp, nil
}

func _store_sql_sync(sql_string string) (*auto.StoreMysqlRsp, error){
	req := auto.StoreMysqlReq{}
	req.Sql = base.T_gostring2mbuf(sql_string)
	req.Ptr = 0

	rsp := auto.StoreMysqlRsp{}

	ret := base.Sync_msg(STORE_THREAD_NAME, auto.STORE_MYSQL_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req), auto.STORE_MYSQL_RSP, int(unsafe.Sizeof(rsp)), unsafe.Pointer(&rsp))
	if ret == false {
		return nil, errors.New("sync error!")
	}

	return &rsp, nil
}

// =====================================================================

func STORESQL(format string, sql ...interface{}) (*tools.Json, error) {
	sql_string := fmt.Sprintf(format, sql...)

	pRsp, err := _store_sql_sync(sql_string)
	if err != nil {
		return nil, err
	}

	if (pRsp.Ret != auto.RetCode_OK) && 
		(pRsp.Ret != auto.RetCode_empty_data) &&
		(pRsp.Ret != auto.RetCode_table_exist) {

		base.DAVELOG("ret:%s on sql:%s", auto.T_auto_RetCode_str(pRsp.Ret), sql_string)

		base.Dave_mfree(pRsp.Data)

		return nil, errors.New("sql failed")
	}

	json_obj, _ := base.T_mbuf2json(pRsp.Data)

	base.Dave_mfree(pRsp.Data)

	base.DAVEDEBUG("sql:%v ret:%v", sql_string, json_obj)

	return json_obj, nil
}

func STORELOAD(json_obj *tools.Json, row int, column int) *tools.Json {
	return json_obj.GetIndex(row).GetIndex(column)
}

func STORELOADRStr(json_obj *tools.Json, row int, column int) string {
	if json_obj == nil {
		return ""
	}

	str_str, err := STORELOAD(json_obj, row, column).String()
	if err != nil {
		base.DAVEDEBUG("err:%v column:%v json:%v", err, column, json_obj)
		return ""
	}
	return str_str
}

func STORELOADStr(json_obj *tools.Json, column int) string {
	if json_obj == nil {
		return ""
	}

	str_str, err := STORELOAD(json_obj, 0, column).String()
	if err != nil {
		base.DAVEDEBUG("err:%v column:%v json:%v", err, column, json_obj)
		return ""
	}
	return str_str
}

func STORELOADSb(json_obj *tools.Json, column int) int {
	if json_obj == nil {
		return 0
	}

	int_str, err := STORELOAD(json_obj, 0, column).String()
	if err != nil {
		base.DAVEDEBUG("err:%v column:%v json:%v", err, column, json_obj)
		return 0
	}
	int_value, _ := strconv.ParseInt(int_str, 10, 64)
	return int(int_value)
}
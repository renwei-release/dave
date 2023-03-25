package option

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"dave/public/auto"

	"dave/product/blockchain/supplier/vsys/store"

	"github.com/mitchellh/mapstructure"
)

type VsysOptionReq struct {
	User_name string `json:"user_name"`
	User_action string `json:"user_action"`
}

type VsysOptionTotalVoucherInqRsp struct {
	Voucher_total_number int `json:"voucher_total_number"`
}

func _vsys_total_voucher_inq() (interface{}, int64) {
	total_number, _, err := vsys_store.Vsys_store_voucher_total(0, 512)
	if err != nil {
		base.DAVELOG("err:%v", err)
		return "", auto.RetCode_empty_data
	}

	rsp := VsysOptionTotalVoucherInqRsp { 
        Voucher_total_number: total_number,
    }

	return rsp, auto.RetCode_OK
}

func _vsys_user_voucher_inq(user_name string) (interface{}, int64) {
	json_obj, _ := vsys_store.Vsys_store_voucher_user(user_name)
	if json_obj == nil {
		return "", auto.RetCode_empty_data
	}
	return json_obj, auto.RetCode_OK
}

func _vsys_user_voucher_assign(user_name string) (interface{}, int64) {
	json_obj, err := vsys_store.Vsys_store_voucher_assign(user_name, 1)
	if err != nil {
		base.DAVELOG("err:%v", err)
		return "", auto.RetCode_empty_data
	}
	return json_obj, auto.RetCode_OK
}

// =====================================================================

func Vsys_option(param interface{}) (interface{}, int64) {
	req := VsysOptionReq{}

	err := mapstructure.Decode(param, &req)
	if err != nil {
		base.DAVELOG("err:%v", err)
		return "", auto.RetCode_Invalid_parameter
	}

	base.DAVEDEBUG("action:%v name:%v",
		req.User_action, req.User_name)

	if req.User_action == "total_voucher_inq" {
		return _vsys_total_voucher_inq()
	} else if req.User_action == "user_voucher_inq" {
		return _vsys_user_voucher_inq(req.User_name)
	} else if req.User_action == "user_voucher_assign" {
		return _vsys_user_voucher_assign(req.User_name)
	} else {
		base.DAVELOG("invalid action, user_action:%v user_name:%v",
			req.User_action, req.User_name)
		return "", auto.RetCode_invalid_option
	}
}
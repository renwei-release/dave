package vsys_store

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/components/store"
	"dave/public/base"

	"encoding/json"
)

const DB_NAME = "VSYS"

const VOUCHER_NAME = "voucher"
const VOUCHER_DISC = "(id int primary key auto_increment," +
	"owner_phone_number varchar(512)," +
	"seed varchar(512)," +
	"wallet_address varchar(512)," +
	"AI_nft_id varchar(512)," +
	"IPFS_hash varchar(512)," +
	"voucher_id varchar(512)," +
	"voucher_nft_id varchar(512)," +
	"voucher_type varchar(512)," +
	"voucher_url varchar(512)," +
	"voucher_expiry varchar(512)," +
	"voucher_status varchar(512)," +
	"voucher_info_id int," +
	"updatetime timestamp default current_timestamp," +
	"constraint uniq_voucher unique(owner_phone_number));"

const VOUCHER_INFO_NAME = "voucher_info"
const VOUCHER_INFO_DISC = "(id int primary key auto_increment," +
	"supplier_name varchar(512)," +
	"trademark_url varchar(512)," +
	"address varchar(512)," +
	"phone_number varchar(512)," +
	"updatetime timestamp default current_timestamp," +
	"constraint voucher_info unique(supplier_name));"

func _store_init() {
	store.STORESQL("CREATE DATABASE %s", DB_NAME)
	store.STORESQL("CREATE TABLE %s.%s %s", DB_NAME, VOUCHER_NAME, VOUCHER_DISC)
	store.STORESQL("CREATE TABLE %s.%s %s", DB_NAME, VOUCHER_INFO_NAME, VOUCHER_INFO_DISC)
}

func _store_exit() {

}

// =====================================================================

func Vsys_store_init() {
	_store_init()
}

func Vsys_store_exit() {
	_store_exit()
}

func Vsys_store_voucher_total(page_id int64, page_number int64) (string, error) {
	json_obj, err := store.STORESQL(
		"SELECT owner_phone_number, voucher_type, voucher_url, voucher_expiry, voucher_status, voucher_info_id FROM %s.%s WHERE voucher_status = \"unused\" LIMIT %d, %d;",
		DB_NAME, VOUCHER_NAME,
		page_id * page_number, page_number)
	if err != nil {
		base.DAVELOG("err:%v", err)
		return "", err
	}

	json_string, err := json.Marshal(json_obj)

	return string(json_string), err
}

func Vsys_store_voucher_user(user_name string) (string, error) {
	json_obj, err := store.STORESQL(
		"SELECT owner_phone_number, voucher_type, voucher_url, voucher_expiry, voucher_status, voucher_info_id FROM %s.%s WHERE owner_phone_number = \"%s\";",
		DB_NAME, VOUCHER_NAME,
		user_name)
	if err != nil {
		base.DAVELOG("err:%v", err)
		return "", err
	}

	json_string, err := json.Marshal(json_obj)

	return string(json_string), err
}

func Vsys_store_voucher_assign(user_name string, assign_number int) (string, error) {
	_, err := store.STORESQL(
		"UPDATE %s.%s SET owner_phone_number = \"%s\", voucher_status = \"used\", updatetime=now() WHERE voucher_status = \"unused\" LIMIT %d;",
		DB_NAME, VOUCHER_NAME,
		user_name,
		assign_number)
	if err != nil {
		base.DAVELOG("err:%v", err)
		return "", err
	}

	return Vsys_store_voucher_user(user_name)
}

func Vsys_store_voucher_add(wallet_address string, voucher_type string, voucher_url string) error {
	_, err := store.STORESQL(
		"INSERT INTO %s.%s" +
		" (wallet_address, voucher_type, voucher_url, voucher_status)" +
		" VALUES (\"%s\", \"%s\", \"%s\", \"unused\");",
		DB_NAME, VOUCHER_NAME,
		wallet_address, voucher_type, voucher_url)
	if err != nil {
		base.DAVELOG("err:%v", err)
		return err
	}

	return nil
}
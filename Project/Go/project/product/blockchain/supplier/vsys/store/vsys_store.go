package vsys_store

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/components/store"
)

const DB_NAME = "VSYS"

const VOUCHER_NAME = "voucher"
const VOUCHER_DISC = "(id int auto_increment," +
	"owner_phone_number varchar(256) primary key," +
	"seed varchar(256)," +
	"wallet_address varchar(256)," +
	"AI_nft_id varchar(256)," +
	"IPFS_hash varchar(256)," +
	"voucher_id varchar(256)," +
	"voucher_nft_id varchar(256)," +
	"voucher_type varchar(256)," +
	"voucher_url varchar(256)," +
	"voucher_expiry varchar(256)," +
	"voucher_status varchar(256)," +
	"updatetime timestamp default current_timestamp);"

func _store_init() {
	store.STORESQL("CREATE DATABASE %s", DB_NAME)
	store.STORESQL("CREATE TABLE %s.%s %s", DB_NAME, VOUCHER_NAME, VOUCHER_DISC)
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
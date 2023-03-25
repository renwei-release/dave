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
	"dave/public/tools"

	"encoding/json"
)

const DB_NAME = "VSYS"

const USER_NAME = "user"
const USER_DISC = "(id int primary key auto_increment," +
	"user_name varchar(512)," +
	"address varchar(512)," +
	"seed varchar(512)," +
	"tokenid varchar(512)," +
	"info varchar(512)," +
	"updatetime timestamp default current_timestamp," +
	"constraint unique(user_name));"

const NFT_NAME = "nft"
const NFT_DISC = "(id int primary key auto_increment," +
	"tokenid varchar(512)," +
	"user_name varchar(512)," +
	"address varchar(512)," +
	"seed varchar(512)," +
	"metadata varchar(512)," +
	"image_url varchar(512)," +
	"ipfs_url varchar(512)," +
	"updatetime timestamp default current_timestamp," +
	"constraint unique(tokenid));"

const VOUCHER_NAME = "voucher"
const VOUCHER_DISC = "(id int primary key auto_increment," +
	"user_name varchar(512)," +
	"voucher_id varchar(512)," +
	"voucher_nft_id varchar(512)," +
	"voucher_type varchar(512)," +
	"voucher_url varchar(512)," +
	"voucher_expiry varchar(512)," +
	"voucher_status varchar(512)," +
	"voucher_info_id int," +
	"updatetime timestamp default current_timestamp," +
	"constraint unique(user_name));"

const VOUCHER_INFO_NAME = "voucher_info"
const VOUCHER_INFO_DISC = "(id int primary key auto_increment," +
	"supplier_name varchar(512)," +
	"trademark_url varchar(512)," +
	"address varchar(512)," +
	"phone_number varchar(512)," +
	"updatetime timestamp default current_timestamp," +
	"constraint unique(supplier_name));"

func _store_init() {
	store.STORESQL("CREATE DATABASE %s", DB_NAME)
	store.STORESQL("CREATE TABLE %s.%s %s", DB_NAME, USER_NAME, USER_DISC)
	store.STORESQL("CREATE TABLE %s.%s %s", DB_NAME, NFT_NAME, NFT_DISC)
	store.STORESQL("CREATE TABLE %s.%s %s", DB_NAME, VOUCHER_NAME, VOUCHER_DISC)
	store.STORESQL("CREATE TABLE %s.%s %s", DB_NAME, VOUCHER_INFO_NAME, VOUCHER_INFO_DISC)
}

func _store_exit() {

}

func _store_voucher_total_number() int {
	json_obj, err := store.STORESQL("SELECT COUNT(id) from %s.%s WHERE voucher_status = \"unused\";", DB_NAME, VOUCHER_NAME)
	if err != nil {
		base.DAVELOG("err:%v", err)
		return 0
	}
	return store.STORELOADSb(json_obj, 0)
}

type VoucherStruct struct {
	User_name string `json:"user_name"`
	Vsys_tokenID string `json:"vsys_tokenid"`
	Image_url string `json:"image_url"`
	Ipfs_url string `json:"ipfs_url"`
	Voucher_nft_id string `json:"voucher_nft_id"`
	Voucher_type string `json:"voucher_type"`
	Voucher_url string `json:"voucher_url"`
	Voucher_expiry string `json:"voucher_expiry"`
	Voucher_status string `json:"voucher_status"`
}

func _store_voucher_struct_to_string(user_name string, voucher_obj *tools.Json) (interface{}, bool) {

	has_voucher := true
	if store.STORELOADStr(voucher_obj, 0) == "" {
		has_voucher = false
	}
	voucher_nft_id := store.STORELOADStr(voucher_obj, 1)
	voucher_type := store.STORELOADStr(voucher_obj, 2)
	voucher_url := store.STORELOADStr(voucher_obj, 3)
	voucher_expiry := store.STORELOADStr(voucher_obj, 4)
	voucher_status := store.STORELOADStr(voucher_obj, 5)

	var tokenid = ""
	var image_url = ""
	var ipfs_url = ""

	_, _, tokenid = Vsys_store_user_inq(user_name)
	if tokenid != "" {
		_, image_url, ipfs_url = Vsys_store_nft_inq(tokenid)
		base.DAVELOG("user:%v token:%v image_url:%v ipfs_url:%v",
			user_name, tokenid, image_url, ipfs_url)
	}

	voucher := VoucherStruct { 
		User_name: user_name,
		Vsys_tokenID: tokenid,
		Image_url: image_url,
		Ipfs_url: ipfs_url,
		Voucher_nft_id: voucher_nft_id,
		Voucher_type: voucher_type,
		Voucher_url: voucher_url,
        Voucher_expiry: voucher_expiry, 
        Voucher_status: voucher_status,
    }

	return voucher, has_voucher
}

func _store_voucher_assign(user_name string, assign_number int) error {
	_, err := store.STORESQL(
		"UPDATE %s.%s SET user_name = \"%s\", voucher_status = \"used\", updatetime=now() WHERE voucher_status = \"unused\" LIMIT %d;",
		DB_NAME, VOUCHER_NAME,
		user_name,
		assign_number)

	if err != nil {
		base.DAVELOG("err:%v", err)
		return err
	}

	return nil
}

// =====================================================================

func Vsys_store_init() {
	_store_init()
}

func Vsys_store_exit() {
	_store_exit()
}

func Vsys_store_user_inq(user_name string) (string, string, string) {
	json_obj, err := store.STORESQL(
		"SELECT address, seed, tokenid FROM %s.%s WHERE user_name = \"%s\";",
		DB_NAME, USER_NAME, user_name)
	
	if err != nil {
		base.DAVELOG("err:%s", err)
		return "", "", ""
	}

	address := store.STORELOADStr(json_obj, 0)
	seed := store.STORELOADStr(json_obj, 1)
	tokenid := store.STORELOADStr(json_obj, 2)

	return address, seed, tokenid
}

func Vsys_store_user_add(user_name string, address string, seed string, tokenid string) {
	exist_address, _, _ :=Vsys_store_user_inq(user_name)

	if exist_address == "" {
		_, err := store.STORESQL(
			"INSERT INTO %s.%s (user_name, address, seed, tokenid) VALUES (\"%s\", \"%s\", \"%s\", \"%s\");",
			DB_NAME, USER_NAME,
			user_name, address, seed, tokenid)
		if err != nil {
			base.DAVELOG("err:%v", err)
		}
	} else {
		_, err := store.STORESQL(
			"UPDATE %s.%s SET address = \"%s\", seed = \"%s\", tokenid = \"%s\", updatetime=now() WHERE user_name = \"%s\";",
			DB_NAME, USER_NAME,
			address, seed, tokenid,
			user_name)
		if err != nil {
			base.DAVELOG("err:%v", err)
		}		
	}
}

func Vsys_store_nft_inq(tokenid string) (string, string, string) {
	json_obj, err := store.STORESQL(
		"SELECT user_name, image_url, ipfs_url FROM %s.%s WHERE tokenid = \"%s\";",
		DB_NAME, NFT_NAME, tokenid)

	if err != nil {
		base.DAVELOG("err:%s", err)
		return "", "", ""
	}

	user_name := store.STORELOADStr(json_obj, 0)
	image_url := store.STORELOADStr(json_obj, 1)
	ipfs_url := store.STORELOADStr(json_obj, 2)

	return user_name, image_url, ipfs_url
}

func Vsys_store_nft_add(tokenid string, user_name string, image_url string, ipfs_url string) {
	exist_user, exist_image, exist_ipfs := Vsys_store_nft_inq(tokenid)
	if exist_user != "" {
		base.DAVELOG("tokenid:%v has been store by user:%v. exist:%v/%v/%v store:%v/%v/%v",
			tokenid, exist_user,
			exist_user, exist_image, exist_ipfs,
			user_name, image_url, ipfs_url)
		return
	}

	address, seed, _ := Vsys_store_user_inq(user_name)
	if address == "" {
		base.DAVELOG("user:%v not exist!", user_name)
		return
	}

	_, err := store.STORESQL(
		"INSERT INTO %s.%s (tokenid, user_name, address, seed, image_url, ipfs_url) VALUES (\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\");",
		DB_NAME, NFT_NAME,
		tokenid, user_name, address, seed,
		image_url, ipfs_url)

	if err != nil {
		base.DAVELOG("err:%v", err)
		return
	}

	Vsys_store_user_add(user_name, address, seed, tokenid)
}

func Vsys_store_voucher_total(page_id int64, page_number int64) (int, string, error) {
	total_number := _store_voucher_total_number()

	json_obj, err := store.STORESQL(
		"SELECT user_name, voucher_nft_id, voucher_type, voucher_url, voucher_expiry, voucher_status FROM %s.%s WHERE voucher_status = \"unused\" LIMIT %d, %d;",
		DB_NAME, VOUCHER_NAME,
		page_id * page_number, page_number)
	if err != nil {
		base.DAVELOG("err:%v", err)
		return total_number, "", err
	}

	json_string, err := json.Marshal(json_obj)

	return total_number, string(json_string), err
}

func Vsys_store_voucher_user(user_name string) (interface{}, bool) {
	json_obj, err := store.STORESQL(
		"SELECT user_name, voucher_nft_id, voucher_type, voucher_url, voucher_expiry, voucher_status FROM %s.%s WHERE user_name = \"%s\";",
		DB_NAME, VOUCHER_NAME,
		user_name)
	if err != nil {
		base.DAVELOG("err:%v", err)
		return "", false
	}

	return _store_voucher_struct_to_string(user_name, json_obj)
}

func Vsys_store_voucher_assign(user_name string, assign_number int) (interface{}, error) {
	voucher_obj, has_voucher := Vsys_store_voucher_user(user_name)
	if has_voucher == true {
		base.DAVELOG("the user:%s has voucher_obj:%v", user_name, voucher_obj)
		return voucher_obj, nil
	}

	err := _store_voucher_assign(user_name, assign_number)
	if err != nil {
		return "", err
	}

	voucher, _ := Vsys_store_voucher_user(user_name)

	return voucher, nil
}

func Vsys_store_voucher_add(voucher_nft_id string, voucher_type string, voucher_url string, voucher_expiry string) error {
	_, err := store.STORESQL(
		"INSERT INTO %s.%s" +
		" (voucher_nft_id, voucher_type, voucher_url, voucher_expiry, voucher_status)" +
		" VALUES (\"%s\", \"%s\", \"%s\", \"%s\", \"unused\");",
		DB_NAME, VOUCHER_NAME,
		voucher_nft_id, voucher_type, voucher_url, voucher_expiry)

	if err != nil {
		base.DAVELOG("err:%v", err)
		return err
	}

	return nil
}
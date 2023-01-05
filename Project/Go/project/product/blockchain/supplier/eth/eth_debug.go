package eth

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/blockchain/supplier/eth/app/nft"
	"dave/product/blockchain/supplier/eth/app/info"
)

func _blockchain_deploy_nft() string {
	ret, address := eth_nft.Eth_deploy_nft("https://github.com/renwei-release/dave", "renwei's token")
	if ret == true {
		return "the NFT address is "+address
	} else {
		return "deploy NFT failed!"
	}
}

func _blockchain_search_nft() {
	eth_nft.Eth_search_nft("0x1E63b953a9774a9EBFC43D4A33b3dC8972FB4346")
}

func _blockchain_address_info() {
	// "0xe8a650bda76f9b81b248662ab0974e5af7776275"
	// "0xF989e900Ac453089A0C0E082FCE037139f355934"
	eth_info.Eth_total_info("0xe8a650bda76f9b81b248662ab0974e5af7776275")
}

func _eth_info() {
	_blockchain_address_info()
	eth_info.Eth_nft_metadata()
}

// =====================================================================

func Eth_debug(debug_req string) string {
	debug_rsp := ""

	if debug_req == "deploy" {
		debug_rsp = _blockchain_deploy_nft()
	} else if debug_req == "search" {
		_blockchain_search_nft()
	} else if debug_req == "info" {
		_eth_info()
	} else {
		debug_rsp = "bad request:"+debug_req
	}

	return debug_rsp
}
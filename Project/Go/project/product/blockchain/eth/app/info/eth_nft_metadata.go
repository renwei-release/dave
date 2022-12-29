package eth_info

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"fmt"
	"net/http"
	"io/ioutil"
)

func _eth_nft_metadata() {
	url := "https://eth-mainnet.g.alchemy.com/nft/v2/docs-demo/getNFTMetadata?contractAddress=0xe785E82358879F061BC3dcAC6f0444462D4b5330&tokenId=44&refreshCache=false"

	req, _ := http.NewRequest("GET", url, nil)

	req.Header.Add("accept", "application/json")

	res, _ := http.DefaultClient.Do(req)

	defer res.Body.Close()
	body, _ := ioutil.ReadAll(res.Body)

	fmt.Println(res)
	fmt.Println(string(body))
}

// =====================================================================

func Eth_nft_metadata() {
	_eth_nft_metadata()
}
package ipfs

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"bytes"
	"dave/public/base"
	"fmt"
	shell "github.com/ipfs/go-ipfs-api"
	"os"
	"strings"
)

func ipfs_cat_data(cid string) {
	sh := shell.NewShell("localhost:5001")

	reader, err := sh.Cat(cid)
	if err != nil {
		fmt.Fprintf(os.Stderr, "error: %s", err)
		os.Exit(1)
	}
	buf := new(bytes.Buffer)
	buf.ReadFrom(reader)
	fmt.Printf("cat the cid:%s -> %s\n", cid, buf)
}

func ipfs_new_build() {
	sh := shell.NewShell("localhost:5001")

	cid, err := sh.Add(strings.NewReader("Hello world"))
	if err != nil {
		fmt.Fprintf(os.Stderr, "error: %s", err)
		os.Exit(1)
	}
	fmt.Printf("add the cid:%s\n", cid)

	ipfs_cat_data(cid)
}

// =====================================================================

func ipfs_debug(debug_data_req string) string {
	base.DAVELOG("%s", debug_data_req)

	if len(debug_data_req) == 0 {
		ipfs_new_build()
	} else {
		ipfs_cat_data(debug_data_req)
	}

	return debug_data_req
}

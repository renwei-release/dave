package ipfs

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"fmt"
	shell "github.com/ipfs/go-ipfs-api"
	"os"
	"strings"
)

// =====================================================================

func ipfs_debug(debug_data_req string) string {
	sh := shell.NewShell("localhost:5001")
	cid, err := sh.Add(strings.NewReader("Hello world"))
	if err != nil {
		fmt.Fprintf(os.Stderr, "error: %s", err)
		os.Exit(1)
	}
	fmt.Printf("added %s", cid)
	return debug_data_req
}

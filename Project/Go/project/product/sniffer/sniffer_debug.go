package sniffer

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"bytes"
	"context"
	"dave/public/base"
	shell "github.com/ipfs/go-ipfs-api"
	"strings"
)

var _MyIPFSNodeServer string = "localhost:5002"

func ipfs_cat_data(cid string) {
	sh := shell.NewShell(_MyIPFSNodeServer)

	base.DAVELOG("start cat cid:%s on %s", cid, _MyIPFSNodeServer)
	reader, err := sh.Cat(cid)
	if err != nil {
		base.DAVELOG("error: %s", err)
		return
	}
	buf := new(bytes.Buffer)
	buf.ReadFrom(reader)
	base.DAVELOG("cat the cid:%s -> %s", cid, buf)
}

func ipfs_new_build() {
	sh := shell.NewShell(_MyIPFSNodeServer)

	cid, err := sh.Add(strings.NewReader("Hello world"))
	if err != nil {
		base.DAVELOG("error: %s", err)
		return
	}
	base.DAVELOG("add the cid:%s", cid)

	ipfs_cat_data(cid)
}

func ipfs_swarm_build() {
	ctx := context.TODO()

	sh := shell.NewShell(_MyIPFSNodeServer)
	base.DAVELOG("connect to ipfs node id!")

	err := sh.SwarmConnect(ctx, "/ip4/1.222.231.117/tcp/41616/p2p/QmPTK2MRQemFkVq5wwR8qn8KSwPWLpVyQfVyHeiv6wqgGL")
	if err != nil {
		base.DAVELOG("SwarmConnect:%v", err)
		return
	}
	base.DAVELOG("swarm connect success!")

	latestTLI, err := sh.Resolve("k51qzi5uqu5dknzcklexyqi8kfhfy8oh8ai7tinrlaa0m6sm6hk1mwangfnafn")
	if err != nil {
		base.DAVELOG("Resolve:%v", err)
		return
	}
	base.DAVELOG("Resolve connect success!")

	cidTLI := strings.Split(latestTLI, "s/")[1]

	base.DAVELOG("get cid :%v", cidTLI)
}

// =====================================================================

func ipfs_debug(debug_data_req string) string {
	base.DAVELOG("%s", debug_data_req)

	if debug_data_req == "new" {
		ipfs_new_build()
	}
	if debug_data_req[0:3] == "cat" {
		ipfs_cat_data(debug_data_req[4:])
	}
	if debug_data_req == "swarm" {
		ipfs_swarm_build()
	}

	return debug_data_req
}

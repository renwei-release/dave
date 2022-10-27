package ipfs

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
	"dave/public/tools"
	shell "github.com/ipfs/go-ipfs-api"
	files "github.com/ipfs/go-ipfs-files"
	"strings"
)

var _MyIPFSNodeServer string = "127.0.0.1:5001"
var _MyIPFSShell * shell.Shell = shell.NewShell(_MyIPFSNodeServer)

func _ipfs_cat_data(cid string) {
	base.DAVELOG("start cat cid:%s on %s", cid, _MyIPFSNodeServer)
	reader, err := _MyIPFSShell.Cat(cid)
	if err != nil {
		base.DAVELOG("error: %s", err)
		return
	}
	buf := new(bytes.Buffer)
	buf.ReadFrom(reader)
	base.DAVELOG("cat the cid:%s -> %s", cid, buf)
}

func _ipfs_add_file(file_name string) {
	base.DAVELOG("add file_name:%s", file_name)

	file_data := tools.T_file_read(file_name)
	if file_data == nil {
		base.DAVELOG("can't read file_name:%s", file_name)
		return
	}

	cid, err := _MyIPFSShell.Add(files.NewBytesFile([]byte(file_data)))
	if err != nil {
		base.DAVELOG("error: %s", err)
		return
	}
	base.DAVELOG("add file_name:%s the cid:%s", file_name, cid)

	_ipfs_cat_data(cid)
}

func _ipfs_add_str(string_data string) {
	cid, err := _MyIPFSShell.Add(strings.NewReader(string_data))
	if err != nil {
		base.DAVELOG("error: %s", err)
		return
	}
	base.DAVELOG("add the cid:%s", cid)

	_ipfs_cat_data(cid)
}

func _ipfs_swarm_build() {
	ctx := context.TODO()

	err := _MyIPFSShell.SwarmConnect(ctx, "/ip4/1.222.231.117/tcp/41616/p2p/QmPTK2MRQemFkVq5wwR8qn8KSwPWLpVyQfVyHeiv6wqgGL")
	if err != nil {
		base.DAVELOG("SwarmConnect:%v", err)
		return
	}
	base.DAVELOG("swarm connect success!")

	latestTLI, err := _MyIPFSShell.Resolve("k51qzi5uqu5dknzcklexyqi8kfhfy8oh8ai7tinrlaa0m6sm6hk1mwangfnafn")
	if err != nil {
		base.DAVELOG("Resolve:%v", err)
		return
	}
	base.DAVELOG("Resolve connect success!")

	cidTLI := strings.Split(latestTLI, "s/")[1]

	base.DAVELOG("get cid :%v", cidTLI)
}

// =====================================================================

func ipfs_cat_data(cid string) {
	_ipfs_cat_data(cid)
}

func ipfs_add_file(file string) {
	_ipfs_add_file(file)
}

func ipfs_add_str(data string) {
	_ipfs_add_str(data)
}

func ipfs_swarm_build() {
	_ipfs_swarm_build()
}
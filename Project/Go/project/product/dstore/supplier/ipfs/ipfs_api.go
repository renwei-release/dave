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
	"time"
)

var _MyIPFSNodeServer string = base.Cfg_get("IPFSServerURL", "18.143.154.81:5001")
var _MyIPNSName string = base.Cfg_get("IPNSName", "self")
var _MyIPFSShell * shell.Shell = shell.NewShell(_MyIPFSNodeServer)
var _update_ipns_enable bool = false

func _ipns_add_cid(cid string) error {
	if _update_ipns_enable == false {
		return nil
	}

	resp, err := _MyIPFSShell.PublishWithDetails("/ipfs/"+cid, _MyIPNSName, time.Second, time.Second, false)
	if err != nil {
		base.DAVELOG("error:%v server:%s name:%s cid:%s pullish failed!",
			err, _MyIPFSNodeServer, _MyIPNSName, cid)
		return err
	}

	base.DAVELOG("ipns name:%s ipfs cid:%s", resp.Name, resp.Value)

	return nil
}

func _ipfs_cat_data(cid string) {
	reader, err := _MyIPFSShell.Cat(cid)
	if err != nil {
		base.DAVELOG("error:%s server:%s", err, _MyIPFSNodeServer)
		return
	}
	buf := new(bytes.Buffer)
	buf.ReadFrom(reader)

	base.DAVELOG("cat the cid:%s->%s from %s", cid, buf, _MyIPFSNodeServer)
}

func _ipfs_add_file(file_name string) string {
	file_data := tools.T_file_read(file_name)
	if file_data == nil {
		base.DAVELOG("can't read file_name:%s", file_name)
		return ""
	}

	cid, err := _MyIPFSShell.Add(files.NewBytesFile([]byte(file_data)))
	if err != nil {
		base.DAVELOG("error:%s server:%s", err, _MyIPFSNodeServer)
		return ""
	}

	base.DAVELOG("add file:%s to:%s the cid:%s", file_name, _MyIPFSNodeServer, cid)

	return cid
}

func _ipfs_add_str(string_data string) string {
	cid, err := _MyIPFSShell.Add(strings.NewReader(string_data))
	if err != nil {
		base.DAVELOG("error:%s server:%s", err, _MyIPFSNodeServer)
		return ""
	}

	base.DAVELOG("add:%s to:%s the cid:%s", string_data, _MyIPFSNodeServer, cid)

	return cid
}

func _ipfs_add_dir(dir string) string {
	cid, err := _MyIPFSShell.AddDir(dir)
	if err != nil {
		base.DAVELOG("error:%s server:%s", err, _MyIPFSNodeServer)
		return ""
	}

	base.DAVELOG("add dir:%s to:%s the cid:%s", dir, _MyIPFSNodeServer, cid)

	return cid
}

func _ipfs_add_bin(bin_data []byte, bin_name string) string {
	bin_path := "/project/ipfs/data/"+tools.T_rand()
	bin_file := bin_path+"/"+bin_name

	err := tools.T_file_write(bin_data, bin_file)
	if err != nil {
		base.DAVELOG("write file:%s failed:%v", bin_file, err)
		return ""
	}

	cid := _ipfs_add_dir(bin_path)

	tools.T_dir_remove(bin_path)

	base.DAVELOG("add bin:%s to:%s the cid:%s", bin_file, _MyIPFSNodeServer, cid)

	return cid
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

	base.DAVELOG("get cid:%v", cidTLI)
}

// =====================================================================

func IPFS_cat_data(cid string) {
	_ipfs_cat_data(cid)
}

func IPFS_add_file(file_name string) string {
	cid := _ipfs_add_file(file_name)

	_ipns_add_cid(cid)

	return cid
}

func IPFS_add_str(string_data string) string {
	cid := _ipfs_add_str(string_data)

	_ipns_add_cid(cid)

	return cid
}

func IPFS_add_dir(dir string) string {
	cid := _ipfs_add_dir(dir)

	_ipns_add_cid(cid)

	return cid
}

func IPFS_add_bin(bin_data []byte, bin_name string) string {
	cid := _ipfs_add_bin(bin_data, bin_name)

	_ipns_add_cid(cid)

	return cid
}

func IPFS_swarm_build() {
	_ipfs_swarm_build()
}
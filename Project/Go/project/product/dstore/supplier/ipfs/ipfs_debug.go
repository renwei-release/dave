package ipfs

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
)

// =====================================================================

func IPFS_debug(debug_data string) string {
	base.DAVELOG("%s", debug_data)

	if debug_data[0:8] == "add_file" {
		IPFS_add_file(debug_data[9:])
	} else if debug_data[0:7] == "add_str" {
		IPFS_add_str(debug_data[8:])
	} else if debug_data[0:7] == "add_dir" {
		IPFS_add_dir(debug_data[8:])
	} else if debug_data[0:3] == "cat" {
		IPFS_cat_data(debug_data[4:])
	} else {
		base.DAVELOG("can't find the command:%s", debug_data)
	}

	return debug_data
}
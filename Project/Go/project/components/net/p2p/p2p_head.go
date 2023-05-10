package p2p
/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
)

// =====================================================================

func P2P_head_set_user() []byte {
	return []byte("USR")
}

func P2P_head_get_user(data []byte) []byte {
	if len(data) <= 3 {
		base.DAVELOG("invalid data:%v/%v", len(data), data)
		return nil
	}
	if string(data[0:3]) != "USR" {
		return nil
	}
	return data[3:]
}

func P2P_head_set_system() []byte {
	return []byte("SYS")
}

func P2P_head_get_system(data []byte) []byte {
	if len(data) <= 3 {
		base.DAVELOG("invalid data:%v/%v", len(data), data)
		return nil
	}
	if string(data[0:3]) != "SYS" {
		return nil
	}
	return data[3:]
}
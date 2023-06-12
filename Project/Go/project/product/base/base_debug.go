package base
/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
)

func __base_memory_debug__() {
	counter := 0
	for {
		if counter > 10000 {
			base.DAVELOG("counter:%d", counter)
			break
		}
		base.Dave_mmalloc(1000)
		counter ++
	}
}

func _base_memory_debug() string {
	for counter := 0; counter < 100; counter++ {
		go __base_memory_debug__()
	}
	return "ok"
}

// =====================================================================

func base_debug(debug_req string) string {
	debug_rsp := ""

	if (len(debug_req) >= 3) && (debug_req[0:3] == "mem") {
		debug_rsp = _base_memory_debug()
	} else {
		debug_rsp = "bad request:"+debug_req
	}

	return debug_rsp
}
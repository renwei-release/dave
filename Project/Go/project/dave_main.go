package main
/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"dave/product"
	"time"
	"strings"
)

const WORKMODE = "Coroutine Inner Loop"

func _main_sleep() {
	for {
		if base.Dave_go_run_state() == true {
			time.Sleep(5 *time.Second)
		} else {
			break
		}
	}
	time.Sleep(time.Second)
}

// =====================================================================

func main() {
	base.Dave_go_init("", WORKMODE, "", product.Product_init, product.Product_exit)
	if find := strings.Contains(WORKMODE, "Inner"); find {
		_main_sleep()
	} else {
		base.Dave_go_running()
	}
	base.Dave_go_exit()
}
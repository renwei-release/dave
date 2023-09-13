package tools

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"time"
)

// =====================================================================

func T_time_current_str() string {
	timeStr:=time.Now().Format("2006-01-02 15:04:05")
	return timeStr
}

func T_time_current_us() uint64 {
	now := time.Now()
	nanos := now.UnixNano()
	micros := nanos / 1000
	return uint64(micros)
}
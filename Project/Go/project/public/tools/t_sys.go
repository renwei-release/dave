package tools

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"runtime"
)

// =====================================================================

func T_sys_go_version() string {
	return runtime.Version()
}
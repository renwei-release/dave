package base
/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"strings"
	"dave/public/tools"
)

var VERSION_PRODUCT = "TEST"
var VERSION_MISC = strings.Replace(tools.T_sys_go_version(), ".", "-", -1)
var VERSION_MAIN = "4"
var VERSION_SUB = "13"
var VERSION_REV = "2"
var VERSION_DATE_TIME = "20230301131253"
var VERSION_LEVEL = "Alpha"

// =====================================================================

func Dave_verno() string {
	return VERSION_PRODUCT + "." + VERSION_MISC + "." + VERSION_MAIN + "." + VERSION_SUB + "." + VERSION_REV + "." + VERSION_DATE_TIME + "." + VERSION_LEVEL
}
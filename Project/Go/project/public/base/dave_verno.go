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

var VERSION_PRODUCT = "SEARCH"
var VERSION_MISC = strings.Replace(tools.T_sys_go_version(), ".", "-", -1)
var VERSION_MAIN = "5"
var VERSION_SUB = "2"
var VERSION_REV = "11"
var VERSION_DATE_TIME = "20261117200633"
var VERSION_LEVEL = "Alpha"
var __BUILD_MAC_ADDRESS__ = "00155D291501"
var __BUILD_HOSTNAME__ = "Renwei-PC-fedora40-root-docker"
var __BUILD_USERNAME__ = "root"

// =====================================================================

func Dave_verno() string {
	return VERSION_PRODUCT + "." + VERSION_MISC + "." + VERSION_MAIN + "." + VERSION_SUB + "." + VERSION_REV + "." + VERSION_DATE_TIME + "." + VERSION_LEVEL
}
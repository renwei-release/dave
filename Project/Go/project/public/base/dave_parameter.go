package base

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

var GLOBALLY_IDENTIFIER_CFG_NAME = "GLOBALLYIDENTIFIER"

func _globally_identifier() string {
	return Cfg_get(GLOBALLY_IDENTIFIER_CFG_NAME, "")
}
var __globally_identifier__ = _globally_identifier()

// =====================================================================

func Globally_identifier() string {
	if __globally_identifier__ == "" {
		__globally_identifier__ = Cfg_get(GLOBALLY_IDENTIFIER_CFG_NAME, "")
	}
	return __globally_identifier__
}
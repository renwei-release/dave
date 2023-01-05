package vsys_core

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"github.com/virtualeconomy/go-v-sdk/vsys"
)

// =====================================================================

func VSYS_init() {
	client_url, nettype :=  VSYS_client_url()

	vsys.InitApi(client_url, nettype)
}
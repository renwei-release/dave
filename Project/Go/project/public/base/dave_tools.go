package base

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/auto"
	"dave/public/tools"
)

// =====================================================================

func T_gostring2mbuf(string_data string) *auto.MBUF {
	mbuf_data := Dave_mmalloc(len(string_data))
	tools.T_cgo_gostring2cbin(mbuf_data.Len, mbuf_data.Payload, string_data)
	return mbuf_data
}

func T_mbuf2gostring(mbuf_data *auto.MBUF) string {
	go_string := tools.T_cgo_cbin2gostring(mbuf_data.Len, mbuf_data.Payload)
	return go_string
}

func T_gobyte2mbuf(byte_data []byte) *auto.MBUF {
	mbuf_data := Dave_mmalloc(len(byte_data))
	tools.T_cgo_gobyte2cbin(mbuf_data.Len, mbuf_data.Payload, byte_data)
	return mbuf_data
}

func T_mbuf2gobyte(mbuf_data *auto.MBUF) []byte {
	go_byte := tools.T_cgo_cbin2gobyte(mbuf_data.Len, mbuf_data.Payload)
	return go_byte
}

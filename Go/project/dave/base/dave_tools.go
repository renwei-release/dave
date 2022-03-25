package base
/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/dave/tools"
)

// =====================================================================

func T_gostring2mbuf(string_data string) * MBUF {
    mbuf_data := Dave_mmalloc(len(string_data))

    tools.T_cgo_gostring2cbin(mbuf_data.Len, mbuf_data.Payload, string_data)

    return mbuf_data
}

func T_mbuf2gostring(mbuf_data *MBUF) string {
    go_string := tools.T_cgo_cbin2gostring(mbuf_data.Len, mbuf_data.Payload)
    return go_string
}
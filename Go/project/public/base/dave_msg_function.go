package base
/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/tools"
)

// =====================================================================

func Dave_msg_process(pMsg * DllMsgBody) {
	msg_src_name := tools.T_cgo_gobyte2gostring(pMsg.msg_src_name[:])
	msg_dst_name := tools.T_cgo_gobyte2gostring(pMsg.msg_dst_name[:])

	msg_id := int(pMsg.msg_id)

	fun, exists := Dave_system_function_table_inq(msg_id)
	if exists {
		fun(msg_src_name, pMsg.msg_src, pMsg.msg_len, pMsg.msg_body)
	} else {
		DAVELOG("unprocess msg, %s->%s:%d", msg_src_name, msg_dst_name, pMsg.msg_id)
	}
}
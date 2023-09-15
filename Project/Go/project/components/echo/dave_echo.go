package echo
/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/auto"
	"dave/public/base"
	"dave/public/tools"
    "unsafe"
)

var HOW_MANY_CYCLES_DO_STATISTICS = 500
var CONCURRENCY_TPS = 5000

var _echo_working bool = false
var _echo_req_counter uint64 = 0

func _echo_api_req_co(gid string, thread string, req auto.MsgIdEchoReq) {
	switch_rand := tools.T_rand_ub() % 4

	if switch_rand == 0 {
		base.Write_co(thread, auto.MSGID_ECHO_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req), auto.MSGID_ECHO_RSP)
	} else if switch_rand == 1 {
		base.Write_qco(thread, auto.MSGID_ECHO_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req), auto.MSGID_ECHO_RSP)
	} else if switch_rand == 2 {
		base.Gid_co(gid, thread, auto.MSGID_ECHO_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req), auto.MSGID_ECHO_RSP)
	} else if switch_rand == 3 {
		base.Gid_qco(gid, thread, auto.MSGID_ECHO_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req), auto.MSGID_ECHO_RSP)
	}
}

func _echo_api_req_msg(gid string, thread string, req auto.MsgIdEchoReq) {
	switch_rand := tools.T_rand_ub() % 4

	if switch_rand == 0 {
		base.Write_msg(thread, auto.MSGID_ECHO_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req))
	} else if switch_rand == 1 {
		base.Write_qmsg(thread, auto.MSGID_ECHO_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req))
	} else if switch_rand == 2 {
		base.Gid_msg(gid, thread, auto.MSGID_ECHO_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req))
	} else if switch_rand == 3 {
		base.Gid_qmsg(gid, thread, auto.MSGID_ECHO_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req))
	}
}

func _echo_api_req(gid string, thread string, req auto.MsgIdEchoReq) {
	_echo_req_counter += 1

	if ((req.Echo.Type == int32(auto.EchoType_random)) && 
		((_echo_req_counter) % 256 == 0) && 
		((tools.T_rand_ub() % 16) == 0)) {
		_echo_api_req_msg(gid, thread, req)
	} else {
		_echo_api_req_msg(gid, thread, req)
	}
}

func _echo_api_rsp(dst uint64, rsp auto.MsgIdEchoRsp) {
	base.Write_msg(dst, auto.MSGID_ECHO_RSP, int(unsafe.Sizeof(rsp)), unsafe.Pointer(&rsp))
}

func _echo_snd_req(gid string, thread string, echo_type int64, getecho auto.MsgIdEcho) {
	req := auto.MsgIdEchoReq{}

	req.Echo = getecho

	req.Echo.Type = int32(echo_type)
	tools.T_cgo_gostring2gobyte(req.Echo.Gid[:], base.Globally_identifier())
	tools.T_cgo_gostring2gobyte(req.Echo.Thread[:], base.Thread_self())

	req.Echo.Echo_req_time = tools.T_time_current_us()

	req.Ptr = 0

	_echo_api_req(gid, thread, req)
}

func _echo_snd_rsp(dst uint64, echo_type int64, getecho auto.MsgIdEcho, ptr uint64) {
	rsp := auto.MsgIdEchoRsp{}

	rsp.Echo = getecho

	rsp.Echo.Type = int32(echo_type)
	tools.T_cgo_gostring2gobyte(rsp.Echo.Gid[:], base.Globally_identifier())
	tools.T_cgo_gostring2gobyte(rsp.Echo.Thread[:], base.Thread_self())

	rsp.Echo.Echo_rsp_time = tools.T_time_current_us()

	rsp.Ptr = ptr

	_echo_api_rsp(dst, rsp)
}

func _echo_random(gid string, thread string, getecho auto.MsgIdEcho, random_send_times uint64) {
	for i := uint64(0); i < random_send_times; i++ {
		_echo_snd_req(gid, thread, auto.EchoType_random, getecho)
	}
}

func _echo_concurrent(gid string, thread string, getecho auto.MsgIdEcho) {
	if getecho.Concurrent_flag == 1 {
		random_send_times := tools.T_rand_ub() % 128
		if random_send_times == 0 {
			random_send_times = 1
		}

		if (tools.T_time_current_us() - getecho.Concurrent_tps_time) >= 1000000 {
			getecho.Concurrent_tps_time = tools.T_time_current_us()
			getecho.Concurrent_tps_counter = 0
		}

		if getecho.Concurrent_tps_counter < uint64(CONCURRENCY_TPS) {
			if getecho.Concurrent_tps_counter + random_send_times > uint64(CONCURRENCY_TPS) {
				random_send_times = uint64(CONCURRENCY_TPS) - getecho.Concurrent_tps_counter
				if random_send_times >= uint64(CONCURRENCY_TPS) {
					random_send_times = 0
				}
			}

			getecho.Concurrent_tps_counter += random_send_times
			getecho.Concurrent_cycle_counter += random_send_times
			getecho.Concurrent_total_counter += random_send_times

			_echo_random(gid, thread, getecho, random_send_times)
		}
	}
}

func _echo_start(concurrent_flag int8) {
	if _echo_working == true {
		base.DAVELOG("The ECHO system is working!")
		return
	}
	_echo_working = true

	if concurrent_flag == 1 {
		base.DAVELOG("start concurrent echo ...")
	} else {
		base.DAVELOG("start single echo ...")
	}

	req := auto.MsgIdEchoReq{}

	req.Echo.Type = int32(auto.EchoType_single)

	tools.T_cgo_gostring2gobyte(req.Echo.Gid[:], base.Globally_identifier())
	tools.T_cgo_gostring2gobyte(req.Echo.Thread[:], base.Thread_self())

	req.Echo.Echo_total_counter = 0
	req.Echo.Echo_total_time = 0

	req.Echo.Echo_cycle_counter = 0
	req.Echo.Echo_cycle_time = 0

	req.Echo.Echo_req_time = tools.T_time_current_us()
	req.Echo.Echo_rsp_time = 0

	req.Echo.Concurrent_flag = concurrent_flag
	req.Echo.Concurrent_tps_time = 0
	req.Echo.Concurrent_tps_counter = 0
	req.Echo.Concurrent_cycle_counter = 0
	req.Echo.Concurrent_total_counter = 0

	copy(req.Echo.Msg[:], "user start echo!")

	req.Ptr = 0

	base.Broadcast_msg("", auto.MSGID_ECHO_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req))
}

func _echo_stop() {
	if _echo_working == false {
		base.DAVELOG("The ECHO system is not working!")
		return
	}
	_echo_working = false
}

func _echo_single_req(src uint64, getecho auto.MsgIdEcho, ptr uint64) {
	gid := tools.T_cgo_gobyte2gostring(getecho.Gid[:])
	thread := tools.T_cgo_gobyte2gostring(getecho.Thread[:])

	_echo_concurrent(gid, thread, getecho)

	_echo_snd_rsp(src, auto.EchoType_single, getecho, ptr)
}

func _echo_single_rsp(src uint64, getecho auto.MsgIdEcho) {
	echo_consume_time := tools.T_time_current_us() - getecho.Echo_req_time

	getecho.Echo_total_counter += 1
	getecho.Echo_total_time += echo_consume_time

	getecho.Echo_cycle_counter += 1
	getecho.Echo_cycle_time += echo_consume_time

	if getecho.Echo_cycle_counter >= uint64(HOW_MANY_CYCLES_DO_STATISTICS) {
		base.DAVELOG("%s/%s C:%ds/%d T:%ds/%d %dus/%dus %d",
			tools.T_cgo_gobyte2gostring(getecho.Gid[:]), tools.T_cgo_gobyte2gostring(getecho.Thread[:]),
			getecho.Echo_cycle_time/1000000, getecho.Echo_cycle_counter,
			getecho.Echo_total_time/1000000, getecho.Echo_total_counter,
			getecho.Echo_cycle_time/(getecho.Echo_cycle_counter*2 + getecho.Concurrent_cycle_counter*2),
			getecho.Echo_total_time/(getecho.Echo_total_counter*2 + getecho.Concurrent_total_counter*2),
			getecho.Echo_total_counter+getecho.Concurrent_total_counter)

		getecho.Echo_cycle_counter = 0
		getecho.Concurrent_cycle_counter = 0
		getecho.Echo_cycle_time = 0
	}

	if _echo_working == true {
		gid := tools.T_cgo_gobyte2gostring(getecho.Gid[:])
		thread := tools.T_cgo_gobyte2gostring(getecho.Thread[:])

		_echo_concurrent(gid, thread, getecho)

		_echo_snd_req(gid, thread, auto.EchoType_single, getecho)
	}
}

func _echo_random_req(src uint64, getecho auto.MsgIdEcho, ptr uint64) {
	_echo_snd_rsp(src, auto.EchoType_random, getecho, ptr)
}

func _echo_random_rsp(src uint64, getecho auto.MsgIdEcho) {
	// Don't do anything.
}

func _echo_req(src uint64, msg_body unsafe.Pointer) {
	pReq := (*auto.MsgIdEchoReq)(msg_body)

	if pReq.Echo.Type == int32(auto.EchoType_start) {
		_echo_start(pReq.Echo.Concurrent_flag)
	} else if pReq.Echo.Type == int32(auto.EchoType_stop) {
		_echo_stop()
	} else if pReq.Echo.Type == int32(auto.EchoType_single) {
		_echo_single_req(src, pReq.Echo, pReq.Ptr)
	} else if pReq.Echo.Type == int32(auto.EchoType_random) {
		_echo_random_req(src, pReq.Echo, pReq.Ptr)
	}
}

func _echo_rsp(src uint64, msg_body unsafe.Pointer) {
	pRsp := (*auto.MsgIdEchoRsp)(msg_body)

	if pRsp.Echo.Type == int32(auto.EchoType_single) {
		_echo_single_rsp(src, pRsp.Echo)
	} else if pRsp.Echo.Type == int32(auto.EchoType_random) {
		_echo_random_rsp(src, pRsp.Echo)
	}
}

func _echo(src uint64, msg_id uint64, msg_body unsafe.Pointer) {
	if msg_id == auto.MSGID_ECHO_REQ {
		_echo_req(src, msg_body)
	} else if msg_id == auto.MSGID_ECHO_RSP {
		_echo_rsp(src, msg_body)
	}
} 

func _echo_req_reg(src_gid string, src_name string, src_id uint64, msg_len uint64, msg_body unsafe.Pointer) {
	_echo(src_id, auto.MSGID_ECHO_REQ, msg_body)
}

func _echo_rsp_reg(src_gid string, src_name string, src_id uint64, msg_len uint64, msg_body unsafe.Pointer) {
	_echo(src_id, auto.MSGID_ECHO_RSP, msg_body)
}

// =====================================================================

func Dave_echo_reg() {
	base.Dave_system_function_table_add(auto.MSGID_ECHO_REQ, _echo_req_reg)
	base.Dave_system_function_table_add(auto.MSGID_ECHO_RSP, _echo_rsp_reg)
}

func Dave_echo_unreg() {
	base.Dave_system_function_table_del(auto.MSGID_ECHO_REQ)
	base.Dave_system_function_table_del(auto.MSGID_ECHO_RSP)
}
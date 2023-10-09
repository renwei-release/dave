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

var S8_ECHO_VALUE = -12
var U8_ECHO_VALUE = 12
var S16_ECHO_VALUE = -1234
var U16_ECHO_VALUE = 1234
var S32_ECHO_VALUE = -6462522
var U32_ECHO_VALUE = 35554553
var S64_ECHO_VALUE = -8376462522
var U64_ECHO_VALUE = 83635554553
var FLOAT_ECHO_VALUE = 12.340000
var DOUBLE_ECHO_VALUE = 123.123000
var VOID_ECHO_VALUE = 739848572524
var STRING_ECHO_VALUE = "string echo!"
var MBUF_ECHO_VALUE = "mbuf echo!"

var _echo_working bool = false
var _echo_req_counter uint64 = 0

func _echo_rpc_reset(echo * auto.MsgIdEcho) {
	echo.S8_echo = byte(S8_ECHO_VALUE)
	echo.U8_echo = byte(U8_ECHO_VALUE)
	echo.S16_echo = int16(S16_ECHO_VALUE)
	echo.U16_echo = uint16(U16_ECHO_VALUE)
	echo.S32_echo = int32(S32_ECHO_VALUE)
	echo.U32_echo = uint32(U32_ECHO_VALUE)
	echo.S64_echo = int64(S64_ECHO_VALUE)
	echo.U64_echo = uint64(U64_ECHO_VALUE)
	echo.Float_echo = float32(FLOAT_ECHO_VALUE)
	echo.Double_echo = float64(DOUBLE_ECHO_VALUE)
	echo.Void_echo = unsafe.Pointer(&VOID_ECHO_VALUE)
	tools.T_cgo_gostring2gobyte(echo.String_echo[:], STRING_ECHO_VALUE)
	echo.Mbuf_echo = base.T_gostring2mbuf(MBUF_ECHO_VALUE)
}

func _echo_rpc_copy(echodst * auto.MsgIdEcho, echosrc * auto.MsgIdEcho) {
	echodst.S8_echo = echosrc.S8_echo
	echodst.U8_echo = echosrc.U8_echo
	echodst.S16_echo = echosrc.S16_echo
	echodst.U16_echo = echosrc.U16_echo
	echodst.S32_echo = echosrc.S32_echo
	echodst.U32_echo = echosrc.U32_echo
	echodst.S64_echo = echosrc.S64_echo
	echodst.U64_echo = echosrc.U64_echo
	echodst.Float_echo = echosrc.Float_echo
	echodst.Double_echo = echosrc.Double_echo
	echodst.Void_echo = echosrc.Void_echo
	tools.T_cgo_gostring2gobyte(echodst.String_echo[:], tools.T_cgo_gobyte2gostring(echosrc.String_echo[:]))
	echodst.Mbuf_echo = base.Dave_mclone(echosrc.Mbuf_echo)
}

func _echo_rpc_verification(echo * auto.MsgIdEcho) {
	if echo.S8_echo != byte(S8_ECHO_VALUE) {
		base.DAVELOG("echo.S8_echo != byte(S8_ECHO_VALUE)")
	}
	if echo.U8_echo != byte(U8_ECHO_VALUE) {
		base.DAVELOG("echo.U8_echo != byte(U8_ECHO_VALUE)")
	}
	if echo.S16_echo != int16(S16_ECHO_VALUE) {
		base.DAVELOG("echo.S16_echo != int16(S16_ECHO_VALUE)")
	}
	if echo.U16_echo != uint16(U16_ECHO_VALUE) {
		base.DAVELOG("echo.U16_echo != uint16(U16_ECHO_VALUE)")
	}
	if echo.S32_echo != int32(S32_ECHO_VALUE) {
		base.DAVELOG("echo.S32_echo != int32(S32_ECHO_VALUE)")
	}
	if echo.U32_echo != uint32(U32_ECHO_VALUE) {
		base.DAVELOG("echo.U32_echo != uint32(U32_ECHO_VALUE)")
	}
	if echo.S64_echo != int64(S64_ECHO_VALUE) {
		base.DAVELOG("echo.S64_echo != int64(S64_ECHO_VALUE)")
	}
	if echo.U64_echo != uint64(U64_ECHO_VALUE) {
		base.DAVELOG("echo.U64_echo != uint64(U64_ECHO_VALUE)")
	}
	if echo.Float_echo != float32(FLOAT_ECHO_VALUE) {
		base.DAVELOG("echo.Float_echo != float32(FLOAT_ECHO_VALUE)")
	}
	if echo.Double_echo != float64(DOUBLE_ECHO_VALUE) {
		base.DAVELOG("echo.Double_echo != float64(DOUBLE_ECHO_VALUE)")
	}
	if echo.Void_echo != unsafe.Pointer(&VOID_ECHO_VALUE) {
		base.DAVELOG("echo.Void_echo != unsafe.Pointer(&VOID_ECHO_VALUE)")
	}
	if tools.T_cgo_gobyte2gostring(echo.String_echo[:]) != STRING_ECHO_VALUE {
		base.DAVELOG("tools.T_cgo_gobyte2gostring(echo.String_echo[:]) != STRING_ECHO_VALUE")
	}
	if base.T_mbuf2gostring(echo.Mbuf_echo) != MBUF_ECHO_VALUE {
		base.DAVELOG("tools.T_cgo_gobyte2gostring(echo.Mbuf_echo.Payload[:]) != MBUF_ECHO_VALUE")
	}
}

func _echo_rpc_clean(echo * auto.MsgIdEcho) {
	base.Dave_mfree(echo.Mbuf_echo)
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

func _echo_api_req_co(gid string, thread string, req auto.MsgIdEchoReq) {
	switch_rand := tools.T_rand_ub() % 4

	pRsp := (*auto.MsgIdEchoRsp)(nil)

	if switch_rand == 0 {
		pRsp = (*auto.MsgIdEchoRsp)(base.Write_co(thread, auto.MSGID_ECHO_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req), auto.MSGID_ECHO_RSP))
	} else if switch_rand == 1 {
		pRsp = (*auto.MsgIdEchoRsp)(base.Write_qco(thread, auto.MSGID_ECHO_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req), auto.MSGID_ECHO_RSP))
	} else if switch_rand == 2 {
		pRsp = (*auto.MsgIdEchoRsp)(base.Gid_co(gid, thread, auto.MSGID_ECHO_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req), auto.MSGID_ECHO_RSP))
	} else if switch_rand == 3 {
		pRsp = (*auto.MsgIdEchoRsp)(base.Gid_qco(gid, thread, auto.MSGID_ECHO_REQ, int(unsafe.Sizeof(req)), unsafe.Pointer(&req), auto.MSGID_ECHO_RSP))
	}

	if pRsp != nil {
		_echo_rpc_clean(&(pRsp.Echo))
	}
}

func _echo_api_req(gid string, thread string, req auto.MsgIdEchoReq) {
	_echo_req_counter += 1

	if ((req.Echo.Type == int32(auto.EchoType_random)) && 
		(_echo_req_counter % 256 == 0) && 
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
	_echo_rpc_reset(&(req.Echo))

	req.Echo.Type = int32(echo_type)
	tools.T_cgo_gostring2gobyte(req.Echo.Gid[:], base.Globally_identifier())
	tools.T_cgo_gostring2gobyte(req.Echo.Thread[:], base.Thread_self())

	req.Echo.Echo_req_time = tools.T_time_current_us()
	req.Echo.Echo_rsp_time = 0

	req.Ptr = 0

	_echo_api_req(gid, thread, req)
}

func _echo_snd_rsp(dst uint64, echo_type int64, getecho auto.MsgIdEcho, ptr uint64) {
	rsp := auto.MsgIdEchoRsp{}

	rsp.Echo = getecho
	_echo_rpc_copy(&(rsp.Echo), &getecho)

	rsp.Echo.Type = int32(echo_type)
	tools.T_cgo_gostring2gobyte(rsp.Echo.Gid[:], base.Globally_identifier())
	tools.T_cgo_gostring2gobyte(rsp.Echo.Thread[:], base.Thread_self())

	rsp.Echo.Echo_req_time = getecho.Echo_req_time
	rsp.Echo.Echo_rsp_time = tools.T_time_current_us()

	rsp.Ptr = ptr

	_echo_api_rsp(dst, rsp)
}

func _echo_random(gid string, thread string, getecho auto.MsgIdEcho, random_send_times uint64) {
	for i := uint64(0); i < random_send_times; i++ {
		_echo_snd_req(gid, thread, auto.EchoType_random, getecho)
	}
}

func _echo_concurrent(gid string, thread string, getecho auto.MsgIdEcho) auto.MsgIdEcho {
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

	return getecho
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

	_echo_rpc_reset(&(req.Echo))

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

	getecho = _echo_concurrent(gid, thread, getecho)

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

		getecho = _echo_concurrent(gid, thread, getecho)

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

	_echo_rpc_clean(&(pReq.Echo))
}

func _echo_rsp(src uint64, msg_body unsafe.Pointer) {
	pRsp := (*auto.MsgIdEchoRsp)(msg_body)

	if pRsp.Echo.Type == int32(auto.EchoType_single) {
		_echo_rpc_verification(&(pRsp.Echo))

		_echo_single_rsp(src, pRsp.Echo)
	} else if pRsp.Echo.Type == int32(auto.EchoType_random) {
		_echo_random_rsp(src, pRsp.Echo)
	}

	_echo_rpc_clean(&(pRsp.Echo))
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
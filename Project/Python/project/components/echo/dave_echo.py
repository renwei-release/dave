# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from public.base import *
from public.tools import *


HOW_MANY_CYCLES_DO_STATISTICS = 500
CONCURRENCY_TPS = 5000

S8_ECHO_VALUE = b'c'
U8_ECHO_VALUE = b'c'
S16_ECHO_VALUE = -1234
U16_ECHO_VALUE = 1234
S32_ECHO_VALUE = -6462522
U32_ECHO_VALUE = 35554553
S64_ECHO_VALUE = -8376462522
U64_ECHO_VALUE = 83635554553
FLOAT_ECHO_VALUE = 12.340000
DOUBLE_ECHO_VALUE = 123.123000
VOID_ECHO_VALUE = 739848572524
STRING_ECHO_VALUE = "string echo!"
MBUF_ECHO_VALUE = "mbuf echo!"

_echo_working = False
_echo_req_counter = 0


def _echo_rpc_reset(echo):
    echo.s8_echo = S8_ECHO_VALUE
    echo.u8_echo = U8_ECHO_VALUE
    echo.s16_echo = S16_ECHO_VALUE
    echo.u16_echo = U16_ECHO_VALUE
    echo.s32_echo = S32_ECHO_VALUE
    echo.u32_echo = U32_ECHO_VALUE
    echo.s64_echo = S64_ECHO_VALUE
    echo.u64_echo = U64_ECHO_VALUE
    echo.float_echo = FLOAT_ECHO_VALUE
    echo.double_echo = DOUBLE_ECHO_VALUE
    echo.string_echo = bytes(STRING_ECHO_VALUE, encoding='utf8')
    echo.mbuf_echo = str_to_mbuf(MBUF_ECHO_VALUE)
    return echo


def _echo_rpc_copy(echodst, echosrc):
    echodst.s8_echo = echosrc.s8_echo
    echodst.u8_echo = echosrc.u8_echo
    echodst.s16_echo = echosrc.s16_echo
    echodst.u16_echo = echosrc.u16_echo
    echodst.s32_echo = echosrc.s32_echo
    echodst.u32_echo = echosrc.u32_echo
    echodst.s64_echo = echosrc.s64_echo
    echodst.u64_echo = echosrc.u64_echo
    echodst.float_echo = echosrc.float_echo
    echodst.double_echo = echosrc.double_echo
    echodst.void_echo = echosrc.void_echo
    echodst.string_echo = echosrc.string_echo
    echodst.mbuf_echo = dave_mclone(echosrc.mbuf_echo)
    return echodst


def _echo_rpc_verification(echo):
    if echo.s8_echo != S8_ECHO_VALUE:
        DAVELOG(f"echo.s8_echo = {echo.s8_echo} != {S8_ECHO_VALUE}")
    if echo.u8_echo != U8_ECHO_VALUE:
        DAVELOG(f"echo.u8_echo = {echo.u8_echo} != {U8_ECHO_VALUE}")
    if echo.s16_echo != S16_ECHO_VALUE:
        DAVELOG(f"echo.s16_echo = {echo.s16_echo} != {S16_ECHO_VALUE}")
    if echo.u16_echo != U16_ECHO_VALUE:
        DAVELOG(f"echo.u16_echo = {echo.u16_echo} != {U16_ECHO_VALUE}")
    if echo.s32_echo != S32_ECHO_VALUE:
        DAVELOG(f"echo.s32_echo = {echo.s32_echo} != {S32_ECHO_VALUE}")
    if echo.u32_echo != U32_ECHO_VALUE:
        DAVELOG(f"echo.u32_echo = {echo.u32_echo} != {U32_ECHO_VALUE}")
    if echo.s64_echo != S64_ECHO_VALUE:
        DAVELOG(f"echo.s64_echo = {echo.s64_echo} != {S64_ECHO_VALUE}")
    if echo.u64_echo != U64_ECHO_VALUE:
        DAVELOG(f"echo.u64_echo = {echo.u64_echo} != {U64_ECHO_VALUE}")
    if echo.string_echo != bytes(STRING_ECHO_VALUE, encoding='utf8'):
        DAVELOG(f"echo.string_echo = {echo.string_echo} != {STRING_ECHO_VALUE}")
    if mbuf_to_str(echo.mbuf_echo) != (MBUF_ECHO_VALUE):
        DAVELOG(f"echo.mbuf_echo = {mbuf_to_str(echo.mbuf_echo)} != {MBUF_ECHO_VALUE}")
    return


def _echo_rpc_clean(echo):
    dave_mfree(echo.mbuf_echo)
    return


def _echo_api_req_co(gid, thread, pReq):
    switch_rand = t_rand_ub() % 4

    if switch_rand == 0:
        write_co(thread, MSGID_ECHO_REQ, pReq)
    elif switch_rand == 1:
        write_qco(thread, MSGID_ECHO_REQ, pReq)
    elif switch_rand == 2:
        gid_co(gid, thread, MSGID_ECHO_REQ, pReq)
    elif switch_rand == 3:
        gid_qco(gid, thread, MSGID_ECHO_REQ, pReq)
    return


def _echo_api_req_msg(gid, thread, pReq):
    switch_rand = t_rand_ub() % 4

    if switch_rand == 0:
        write_msg(thread, MSGID_ECHO_REQ, pReq)
    elif switch_rand == 1:
        write_qmsg(thread, MSGID_ECHO_REQ, pReq)
    elif switch_rand == 2:
        gid_msg(gid, thread, MSGID_ECHO_REQ, pReq)
    elif switch_rand == 3:
        gid_qmsg(gid, thread, MSGID_ECHO_REQ, pReq)
    return


def _echo_api_req(gid, thread, pReq):
    global _echo_req_counter
    _echo_req_counter += 1

    if (pReq.contents.echo.type == EchoType_random) and \
        (_echo_req_counter % 256 == 0) and \
        ((t_rand_ub() % 16) == 0):
        _echo_api_req_co(gid, thread, pReq)
    else:
        _echo_api_req_msg(gid, thread, pReq)
    return


def _echo_api_rsp(dst, pRsp):
    write_msg(dst, MSGID_ECHO_RSP, pRsp)
    return


def _echo_snd_req(gid, thread, echo_type, getecho):
    pReq = thread_msg(MsgIdEchoReq)

    pReq.contents.echo = getecho
    pReq.contents.echo = _echo_rpc_reset(pReq.contents.echo)

    pReq.contents.echo.type = echo_type
    pReq.contents.echo.gid = bytes(globally_identifier(), encoding='utf8')
    pReq.contents.echo.thread = bytes(thread_self(), encoding='utf8')

    pReq.contents.echo.echo_req_time = t_time_current_us()
    pReq.contents.echo.echo_rsp_time = 0

    pReq.contents.ptr = None

    _echo_api_req(gid, thread, pReq)
    return


def _echo_snd_rsp(dst, echo_type, getecho, ptr):
    pRsp = thread_msg(MsgIdEchoRsp)

    pRsp.contents.echo = getecho
    pRsp.contents.echo = _echo_rpc_copy(pRsp.contents.echo, getecho)

    pRsp.contents.echo.type = echo_type
    pRsp.contents.echo.gid = bytes(globally_identifier(), encoding='utf8')
    pRsp.contents.echo.thread = bytes(thread_self(), encoding='utf8')

    pRsp.contents.echo.echo_req_time = getecho.echo_req_time
    pRsp.contents.echo.echo_rsp_time = t_time_current_us()

    pRsp.contents.ptr = ptr

    _echo_api_rsp(dst, pRsp)
    return


def _echo_random(gid, thread, getecho, random_send_times):
    for i in range(random_send_times):
        _echo_snd_req(gid, thread, EchoType_random, getecho)
    return


def _echo_concurrent(gid, thread, getecho):
    if getecho.concurrent_flag == True:
        random_send_times = t_rand_ub() % 128
        if random_send_times == 0:
            random_send_times = 1

        if t_time_current_us() - getecho.concurrent_tps_time > 1000000:
            getecho.concurrent_tps_time = t_time_current_us()
            getecho.concurrent_tps_counter = 0
        
        if getecho.concurrent_tps_counter < CONCURRENCY_TPS:
            if getecho.cuncurrent_tps_counter + random_send_times > CONCURRENCY_TPS:
                random_send_times = CONCURRENCY_TPS - getecho.concurrent_tps_counter
                if random_send_times >= CONCURRENCY_TPS:
                    random_send_times = 0

                getecho.concurrent_tps_counter += random_send_times
                getecho.concurrent_cycle_counter += random_send_times
                getecho.concurrent_total_counter += random_send_times

                _echo_random(gid, thread, getecho, random_send_times)
    return getecho


def _echo_start(concurrent_flag):
    global _echo_working
    if _echo_working == True:
        DAVELOG("The ECHO system is working!")
        return
    _echo_working = True

    if concurrent_flag:
        DAVELOG("start concurrent echo ...")
    else:
        DAVELOG("start single echo ...")

    pReq = thread_msg(MsgIdEchoReq)

    pReq.contents.echo.type = EchoType_single

    pReq.contents.echo.gid = bytes(globally_identifier(), encoding='utf8')
    pReq.contents.echo.thread = bytes(thread_self(), encoding='utf8')

    pReq.contents.echo.echo_total_counter = 0
    pReq.contents.echo.echo_total_time = 0

    pReq.contents.echo.echo_cycle_counter = 0
    pReq.contents.echo.echo_cycle_time = 0
    pReq.contents.echo.echo_req_time = t_time_current_us()
    pReq.contents.echo.echo_rsp_time = 0

    pReq.contents.echo.concurrent_flag = concurrent_flag
    pReq.contents.echo.concurrent_tps_time = 0
    pReq.contents.echo.concurrent_tps_counter = 0
    pReq.contents.echo.concurrent_cycle_counter = 0
    pReq.contents.echo.concurrent_total_counter = 0

    pReq.echo = _echo_rpc_reset(pReq.contents.echo)

    pReq.contents.ptr = None

    broadcast_msg("", MSGID_ECHO_REQ, pReq)
    return


def _echo_stop():
    global _echo_working
    if _echo_working == False:
        DAVELOG("The ECHO system is not working!")
        return
    _echo_working = False
    return


def _echo_single_req(src, getecho, ptr):
    getecho = _echo_concurrent(getecho.gid, getecho.thread, getecho)

    _echo_snd_rsp(src, EchoType_single, getecho, ptr)
    return


def _echo_single_rsp(src, getecho):
    echo_consume_time = t_time_current_us() - getecho.echo_req_time

    getecho.echo_total_counter += 1
    getecho.echo_total_time += echo_consume_time

    getecho.echo_cycle_counter += 1
    getecho.echo_cycle_time += echo_consume_time

    if getecho.echo_cycle_counter >= HOW_MANY_CYCLES_DO_STATISTICS:
        DAVELOG(f"{getecho.gid}/{getecho.thread} "\
                f"C:{int(getecho.echo_cycle_time/1000000)}s/{getecho.echo_cycle_counter} "\
                f"T:{int(getecho.echo_total_time/1000000)}s/{getecho.echo_total_counter} "\
                f"{int(getecho.echo_cycle_time/(getecho.echo_cycle_counter*2 + getecho.concurrent_cycle_counter*2))}us/{int(getecho.echo_total_time/(getecho.echo_total_counter*2 + getecho.concurrent_total_counter*2))}us "\
                f"{getecho.echo_total_counter+getecho.concurrent_total_counter}")

        getecho.echo_cycle_counter = 0
        getecho.concurrent_cycle_counter = 0
        getecho.echo_cycle_time = 0

    global _echo_working
    if _echo_working == True:
        getecho = _echo_concurrent(getecho.gid, getecho.thread, getecho)

        _echo_snd_req(getecho.gid, getecho.thread, EchoType_single, getecho)

    return


def _echo_random_req(src, getecho, ptr):
    _echo_snd_rsp(src, EchoType_random, getecho, ptr)
    return


def _echo_random_rsp(src, getecho):
    # Don't do anything.
    return


def _echo_req(src, msg_len, msg_body):
    pReq = struct_copy(MsgIdEchoReq, msg_body, msg_len)

    if pReq.echo.type == EchoType_start:
        _echo_start(pReq.echo.concurrent_flag)
    elif pReq.echo.type == EchoType_stop:
        _echo_stop()
    elif pReq.echo.type == EchoType_single:
        _echo_single_req(src, pReq.echo, pReq.ptr)
    elif pReq.echo.type == EchoType_random:
        _echo_random_req(src, pReq.echo, pReq.ptr)
		
    _echo_rpc_clean(pReq.echo)
    return


def _echo_rsp(src, msg_len, msg_body):
    pRsp = struct_copy(MsgIdEchoRsp, msg_body, msg_len)

    if pRsp.echo.type == EchoType_single:
        _echo_rpc_verification(pRsp.echo)

        _echo_single_rsp(src, pRsp.echo)
    elif pRsp.echo.type == EchoType_random:
        _echo_random_rsp(src, pRsp.echo)
		
    _echo_rpc_clean(pRsp.echo)
    return


def _echo(src, msg_id, msg_len, msg_body):
    if msg_id == MSGID_ECHO_REQ:
        _echo_req(src, msg_len, msg_body)
    elif msg_id == MSGID_ECHO_RSP:
        _echo_rsp(src, msg_len, msg_body)
    return


def _echo_req_reg(src_name, src_id, msg_len, msg_body):
    _echo(src_id, MSGID_ECHO_REQ, msg_len, msg_body)
    return


def _echo_rsp_reg(src_name, src_id, msg_len, msg_body):
    _echo(src_id, MSGID_ECHO_RSP, msg_len, msg_body)
    return


# =====================================================================


def dave_echo_reg():
    dave_system_function_table_add(MSGID_ECHO_REQ, _echo_req_reg)
    dave_system_function_table_add(MSGID_ECHO_RSP, _echo_rsp_reg)
    return

def dave_echo_unreg():
    dave_system_function_table_del(MSGID_ECHO_REQ)
    dave_system_function_table_del(MSGID_ECHO_RSP)
    return
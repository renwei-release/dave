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


_echo_working = False
_echo_req_counter = 0


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

    pRsp.contents.echo.type = echo_type
    pRsp.contents.echo.gid = bytes(globally_identifier(), encoding='utf8')
    pRsp.contents.echo.thread = bytes(thread_self(), encoding='utf8')

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

    pReq.contents.echo.msg = bytes("user start echo!", encoding='utf8')

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
    return


def _echo_rsp(src, msg_len, msg_body):
    pRsp = struct_copy(MsgIdEchoRsp, msg_body, msg_len)

    if pRsp.echo.type == EchoType_single:
        _echo_single_rsp(src, pRsp.echo)
    elif pRsp.echo.type == EchoType_random:
        _echo_random_rsp(src, pRsp.echo)
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
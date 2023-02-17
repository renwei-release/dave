# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from public import *


HTTP_THREAD_NAME=b'distributor'


def _http_register_path_rsp(src_name, src_id, msg_len, msg_body):
    pRsp = struct_copy(HTTPListenRsp, msg_body, msg_len)

    DAVELOG(f"register ret:{t_auto_RetCode_str(pRsp.ret)} path:{pRsp.path}")
    return


def _http_register_path(path):
    pReq = thread_msg(HTTPListenReq)
    pReq.contents.path = bytes(path, encoding='utf8')
    pReq.contents.ptr = None

    dave_system_function_table_add(HTTPMSG_LISTEN_RSP, _http_register_path_rsp)
    broadcast_msg(HTTP_THREAD_NAME, HTTPMSG_LISTEN_REQ, pReq)

    return


def _http_register_auto_rsp(src_name, src_id, msg_len, msg_body):
    pRsp = struct_copy(HTTPListenAutoCloseRsp, msg_body, msg_len)

    if pRsp.ret != RetCode_OK:
        DAVELOG(f"register ret:{t_auto_RetCode_str(pRsp.ret)} path:{pRsp.path}")
    return


def _http_register_auto(path, time):
    pReq = thread_msg(HTTPListenAutoCloseReq)
    pReq.contents.path = bytes(path, encoding='utf8')
    pReq.contents.listening_seconds_time = time
    pReq.contents.ptr = None

    dave_system_function_table_add(HTTPMSG_LISTEN_AUTO_CLOSE_RSP, _http_register_auto_rsp)
    broadcast_msg(HTTP_THREAD_NAME, HTTPMSG_LISTEN_AUTO_CLOSE_REQ, pReq)

    return


# =====================================================================


def http_register(path, time, recv_req_function):
    dave_system_function_table_add(HTTPMSG_RECV_REQ, recv_req_function)

    if time == 0:
        _http_register_path(path)
    else:
        _http_register_auto(path, time)


def http_recv_rsp(dst, ret, data, ptr):
    pRsp = thread_msg(HTTPRecvRsp)
    pRsp.contents.ret = ret
    if isinstance(data, str):
        pRsp.contents.content = str_to_mbuf(data)
    else:
        pRsp.contents.content = byte_to_mbuf(data)
    pRsp.contents.ptr = ptr
    write_msg(dst, HTTPMSG_RECV_RSP, pRsp)
    return
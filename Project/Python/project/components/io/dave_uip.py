# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from public import *


UIP_THREAD_NAME=b'uip'

register_recv_req_function = {}


def _uip_register_rsp(src_name, src_id, msg_len, msg_body):
    pRsp = struct_copy(UIPRegisterRsp, msg_body, msg_len)

    if pRsp.ret != RetCode_OK:
        DAVELOG(f"register ret:{t_auto_RetCode_str(pRsp.ret)}"\
                f" method:{str(pRsp.method[0], encoding='utf8')}")
    return


def _uip_recv_req(src_name, src_id, msg_len, msg_body):
    pReq = struct_copy(UIPDataRecvReq, msg_body, msg_len)

    method = str(pReq.method, encoding='utf8')

    req_function = register_recv_req_function.get(method, None)
    if req_function is None:
        DAVELOG(f"method:{pReq.method} not register")
        uip_recv_rsp(src_id, RetCode_OK, pReq.method, "", pReq.ptr)
        return
    req_function(src_name, src_id, msg_len, msg_body)
    return


# =====================================================================


def uip_register(method, recv_req_function):
    pReq = thread_msg(UIPRegisterReq)
    t_copy_str_to_array(pReq.contents.method[0], method)
    pReq.contents.ptr = None

    register_recv_req_function[method] = recv_req_function

    dave_system_function_table_add(UIP_REGISTER_RSP, _uip_register_rsp)
    dave_system_function_table_add(UIP_DATA_RECV_REQ, _uip_recv_req)

    broadcast_msg(UIP_THREAD_NAME, UIP_REGISTER_REQ, pReq)
    return True


def uip_recv_rsp(dst, ret, method, data, ptr):
    pRsp = thread_msg(UIPDataRecvRsp)
    pRsp.contents.ret = ret
    pRsp.contents.method = method
    if isinstance(data, str):
        pRsp.contents.data = str_to_mbuf(data)
    else:
        pRsp.contents.data = byte_to_mbuf(data)
    pRsp.contents.ptr = ptr
    write_msg(dst, UIP_DATA_RECV_RSP, pRsp)
    return
# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from public import *


UIP_THREAD_NAME=b'uip'


def _uip_register_rsp(src_name, src_id, msg_len, msg_body):
    pRsp = struct_copy(UIPRegisterRsp, msg_body, msg_len)

    DAVELOG(f"register ret:{t_auto_RetCode_str(pRsp.ret)} method:{str(pRsp.method[0], encoding='utf8')}")
    return


# =====================================================================


def uip_register(method, recv_req_function):
    pReq = thread_msg(UIPRegisterReq)
    t_copy_str_to_array(pReq.contents.method[0], method)
    pReq.contents.ptr = None

    dave_system_function_table_add(UIP_REGISTER_RSP, _uip_register_rsp)
    dave_system_function_table_add(UIP_DATA_RECV_REQ, recv_req_function)

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
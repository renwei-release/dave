# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.06.07.
# ================================================================================
#
import time
import datetime
from public import *


def busybox_MAINMSG_PYTHON_RSP(dst, ptr, time, rsp_data):
    pRsp = thread_msg(MainMsgPythonRsp)
    pRsp.contents.ret = ERRCODE_OK
    pRsp.contents.time = time
    if rsp_data != None:
        pRsp.contents.rsp_data = byte_to_mbuf(rsp_data)
    else:
        pRsp.contents.rsp_data = None
    pRsp.contents.ptr = ptr
    write_msg(dst, MAINMSG_PYTHON_RSP, pRsp)
    return


def busybox_MAINMSG_PYTHON_REQ(function_table, msg_src, msg_len, msg_body):
    pReq = struct_copy(MainMsgPythonReq, msg_body, msg_len)
    fun = function_table.get(pReq.fun, None)
    start_time = time.time()
    if fun != None:
        rsp_data = fun(pReq.opt_param, pReq.file_path, mbuf_to_byte(pReq.req_data))
    else:
        DAVELOG("unprocess fun:{} opt_param:{}".format(pReq.fun, pReq.opt_param))
        rsp_data = None
    stop_time = time.time()
    busybox_MAINMSG_PYTHON_RSP(msg_src, pReq.ptr, int(round((stop_time - start_time) * 1000000)), rsp_data)
    dave_mfree(pReq.req_data)
    return


def busybox_APPMSG_FUNCTION_REGISTER_RSP(src_name, src_id, msg_len, msg_body):
    pRsp = struct_copy(AppMsgFunctionRegRsp, msg_body, msg_len)
    DAVELOG(f'ret:{pRsp.ret} thread:{pRsp.thread_name} function:{pRsp.function_id}')
    return


def busybox_APPMSG_FUNCTION_REGISTER_REQ(thread_name, function_id):
    DAVELOG(f'thread:{thread_name} function:{function_id}')
    pReq = thread_msg(AppMsgFunctionRegReq)
    pReq.contents.thread_name = dave_product()
    pReq.contents.function_id = function_id
    broadcast_msg(thread_name, APPMSG_FUNCTION_REGISTER_REQ, pReq)
    dave_system_function_table_add(APPMSG_FUNCTION_REGISTER_RSP, busybox_APPMSG_FUNCTION_REGISTER_RSP)
    return

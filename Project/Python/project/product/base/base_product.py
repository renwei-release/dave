# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from public import *
from .base_debug import *


def fun_MSGID_DEBUG_REQ(src_name, src_id, msg_len, msg_body):
    pReq = struct_copy(DebugReq, msg_body, msg_len)

    pRsp = thread_msg(DebugRsp)
    pRsp.contents.msg = bytes(base_debug(str(pReq.msg, encoding = "utf-8")), encoding='utf8')
    pRsp.contents.ptr = pReq.ptr

    write_msg(src_id, MSGID_DEBUG_RSP, pRsp)
    return


def fun_MSGID_REMOTE_THREAD_ID_READY(src_name, src_id, msg_len, msg_body):
    pReady = struct_copy(ThreadRemoteIDReadyMsg, msg_body, msg_len)
    DAVELOG(f'{pReady.remote_thread_id} {pReady.remote_thread_name} {type(pReady.globally_identifier)}{pReady.globally_identifier}')
    return


def fun_MSGID_REMOTE_THREAD_ID_REMOVE(src_name, src_id, msg_len, msg_body):
    pRemove = struct_copy(ThreadRemoteIDRemoveMsg, msg_body, msg_len)
    DAVELOG(f'{pRemove.remote_thread_id} {pRemove.remote_thread_name} {type(pRemove.globally_identifier)}{pRemove.globally_identifier}')
    return


def fun_cfg_update(name_ptr, name_len, value_ptr, value_len):
    DAVELOG(f'{name_len}/{name_ptr}:{value_len}{value_ptr}')
    return


# =====================================================================


def dave_product_init():
    dave_system_function_table_add(MSGID_DEBUG_REQ, fun_MSGID_DEBUG_REQ)
    dave_system_function_table_add(MSGID_REMOTE_THREAD_ID_READY, fun_MSGID_REMOTE_THREAD_ID_READY)
    dave_system_function_table_add(MSGID_REMOTE_THREAD_ID_REMOVE, fun_MSGID_REMOTE_THREAD_ID_REMOVE)
    cfg_reg("BaseCfgTest", fun_cfg_update)
    return


def dave_product_exit():
    dave_system_function_table_del(MSGID_DEBUG_REQ)
    dave_system_function_table_del(MSGID_REMOTE_THREAD_ID_READY)
    dave_system_function_table_del(MSGID_REMOTE_THREAD_ID_REMOVE)
    return
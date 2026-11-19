# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import threading
from ..auto import *
from ..tools.t_rand import t_rand
from .dave_dll import *
from .dave_thread import *
from .dave_tools import *
from .dave_log import *


def _general_data_process(general_data):
    general_bin = general_data.get('__BIN_DATA__', None)

    if general_bin != None:
        del general_data["__BIN_DATA__"]

        if type(general_bin) == dict:
            general_bin = dict_to_mbuf(general_bin)
        elif type(general_bin) == str:
            general_bin = str_to_mbuf(general_bin)
        elif type(general_bin) == bytes:
            general_bin = byte_to_mbuf(general_bin)
        else:
            DAVELOG(f"Wrong general_bin type:{type(general_bin)}!!!")
            general_bin = None

    if general_data != None:
        if type(general_data) == dict:
            general_data = dict_to_mbuf(general_data)
        elif type(general_data) == str:
            general_data = str_to_mbuf(general_data)
        elif type(general_data) == bytes:
            general_data = byte_to_mbuf(general_data)
        else:
            DAVELOG(f"Wrong general_data type:{type(general_data)}!!!")
            general_data = None

    return general_data, general_bin


def _general_call_process(dst_gid, call_dst, general_type, general_data):
    if type(call_dst) == str:
        if thread_id(call_dst) == -1:
            return {}

    pReq = thread_msg(GeneralReq)

    if type(general_type) == str:
        general_type = bytes(general_type, encoding='utf8')
    else:
        DAVELOG(f"Wrong general_type type:{general_type}!!!")
        general_type = None

    if general_type == None:
        general_type = b'None'

    general_data, general_bin = _general_data_process(general_data)

    pReq.contents.general_type = general_type
    pReq.contents.general_data = general_data
    pReq.contents.general_bin = general_bin
    pReq.contents.send_req_us_time = t_time_current_us()
    pReq.contents.ptr = None

    if dave_python_work_mode() == b'Coroutine Outer Loop':
        if dst_gid == None:
            pRsp = write_co(call_dst, MSGID_GENERAL_REQ, pReq, MSGID_GENERAL_RSP, GeneralRsp)
        else:
            pRsp = gid_co(dst_gid, call_dst, MSGID_GENERAL_REQ, pReq, MSGID_GENERAL_RSP, GeneralRsp)
    else:
        if dst_gid == None:
            pRsp = sync_msg(call_dst, MSGID_GENERAL_REQ, pReq, MSGID_GENERAL_RSP, GeneralRsp)
        else:
            pRsp = gid_sync(dst_gid, call_dst, MSGID_GENERAL_REQ, pReq, MSGID_GENERAL_RSP, GeneralRsp)

    if pRsp == None:
        return {}
    if pRsp.general_data == None:
        return {}

    general_result = mbuf_to_dict(pRsp.general_data)
    if pRsp.general_bin != None:
        general_result['__BIN_DATA__'] = mbuf_to_byte(pRsp.general_bin)

    dave_mfree(pRsp.general_bin)
    dave_mfree(pRsp.general_data)

    return general_result


__general_task__ = {}
__general_task_lock__ = threading.Lock()
__general_task_cond__ = threading.Condition(__general_task_lock__)
__GENERAL_PENDING__ = object()


def _general_loop(src_name, src_id, msg_len, msg_body):
    global __general_task__
    global __general_task_lock__
    global __general_task_cond__

    pLoop = struct_copy(MsgInnerLoop, msg_body, msg_len)

    params = mbuf_to_dict(pLoop.param, free_mbuf=True)

    task_id = params.get("task_id")
    call_dst = params.get("call_dst")
    dst_gid = params.get("dst_gid")
    general_type = params.get("general_type")
    general_data = params.get("general_data")

    general_result = _general_call_process(dst_gid, call_dst, general_type, general_data)

    with __general_task_cond__:
        __general_task__[task_id] = general_result
        __general_task_cond__.notify_all()
    return


# =====================================================================


def dave_call_general(call_dst, general_type, general_data, dst_gid=None):
    global __general_task__
    global __general_task_lock__
    global __general_task_cond__
    global __GENERAL_PENDING__

    with __general_task_cond__:
        task_id = t_rand()
        while task_id in __general_task__:
            task_id = t_rand()
        __general_task__[task_id] = __GENERAL_PENDING__

    inner_loop(_general_loop, {
        "task_id": task_id,
        "call_dst": call_dst,
        "dst_gid": dst_gid,
        "general_type": general_type,
        "general_data": general_data
    }, GENERAL_LOOP_MSG)

    with __general_task_cond__:
        while __general_task__.get(task_id) is __GENERAL_PENDING__:
            __general_task_cond__.wait()
        general_result = __general_task__.pop(task_id, {})

    if general_result == {}:
        DAVELOG(f"general call returned empty result for task_id:{task_id} call_dst:{call_dst} general_type:{general_type}")
    return general_result


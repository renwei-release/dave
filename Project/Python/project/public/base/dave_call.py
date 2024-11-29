# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from .dave_thread import *
from .dave_tools import *
from .dave_log import *


# =====================================================================


def dave_call_general(call_dst, general_type, general_data):
    if type(call_dst) == str:
        if thread_id(call_dst) == -1:
            return None

    pReq = thread_msg(GeneralReq)

    if type(general_type) == str:
        general_type = bytes(general_type, encoding='utf8')
    else:
        DAVELOG(f"Wrong general_type type:{general_type}!!!")
        general_type = None

    if type(general_data) == dict:
        general_data = dict_to_mbuf(general_data)
    elif type(general_data) == str:
        general_data = str_to_mbuf(general_data)
    elif type(general_data) == bytes:
        general_data = byte_to_mbuf(general_data)
    else:
        DAVELOG(f"Wrong general_data type:{type(general_data)}!!!")
        general_data = None

    pReq.contents.general_type = general_type
    pReq.contents.general_data = general_data
    pReq.contents.send_req_us_time = t_time_current_us()
    pReq.ptr = None

    pRsp = sync_msg(call_dst, MSGID_GENERAL_REQ, pReq, MSGID_GENERAL_RSP, GeneralRsp)

    if pRsp == None:
        return None
    if pRsp.general_data == None:
        return None

    result = mbuf_to_dict(pRsp.general_data)

    dave_mfree(pRsp.general_data)

    return result
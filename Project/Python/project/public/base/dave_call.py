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

    if general_type == None:
        general_type = b'None'

    general_data, general_bin = _general_data_process(general_data)

    pReq.contents.general_type = general_type
    pReq.contents.general_data = general_data
    pReq.contents.general_bin = general_bin
    pReq.contents.send_req_us_time = t_time_current_us()
    pReq.contents.ptr = None

    pRsp = sync_msg(call_dst, MSGID_GENERAL_REQ, pReq, MSGID_GENERAL_RSP, GeneralRsp)

    if pRsp == None:
        return None
    if pRsp.general_data == None:
        return None

    general_result = mbuf_to_dict(pRsp.general_data)
    if pRsp.general_bin != None:
        general_result['__BIN_DATA__'] = mbuf_to_byte(pRsp.general_bin)

    dave_mfree(pRsp.general_bin)
    dave_mfree(pRsp.general_data)

    return general_result


def dave_call_general_req(call_dst, general_type, general_data):
    if type(call_dst) == str:
        if thread_id(call_dst) == -1:
            return None

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

    write_msg(call_dst, MSGID_GENERAL_REQ, pReq)
    return


def dave_call_general_rsp(call_dst, general_type, general_data):
    if type(call_dst) == str:
        if thread_id(call_dst) == -1:
            return None

    pRsp = thread_msg(GeneralRsp)

    if type(general_type) == str:
        general_type = bytes(general_type, encoding='utf8')
    else:
        DAVELOG(f"Wrong general_type type:{general_type}!!!")
        general_type = None

    if general_type == None:
        general_type = b'None'

    general_data, general_bin = _general_data_process(general_data)

    pRsp.contents.general_type = general_type
    pRsp.contents.general_data = general_data
    pRsp.contents.general_bin = general_bin
    pRsp.contents.send_rsp_us_time = t_time_current_us()
    pRsp.contents.ptr = None

    write_msg(call_dst, MSGID_GENERAL_RSP, pRsp)
    return

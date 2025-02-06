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


def dave_call_rtc_translation_start(call_dst, translation_id, src_lang, dst_lang):
    if type(call_dst) == str:
        if thread_id(call_dst) == -1:
            return None

    if type(translation_id) == str:
        translation_id = bytes(translation_id, encoding='utf8')
    if type(src_lang) == str:
        src_lang = bytes(src_lang, encoding='utf8')
    if type(dst_lang) == str:
        dst_lang = bytes(dst_lang, encoding='utf8')

    pReq = thread_msg(RTCTranslationStartReq)

    pReq.contents.translation_id = translation_id
    pReq.contents.src_lang = src_lang
    pReq.contents.dst_lang = dst_lang
    pReq.contents.ptr = None

    pRsp = sync_msg(call_dst, RTC_TRANSLATION_START_REQ, pReq, RTC_TRANSLATION_START_RSP, RTCTranslationStartRsp)
    if pRsp == None:
        return None
    return pRsp.gid


def dave_call_rtc_translation_stop(call_dst, translation_id):
    if type(call_dst) == str:
        if thread_id(call_dst) == -1:
            return False

    if type(translation_id) == str:
        translation_id = bytes(translation_id, encoding='utf8')

    pReq = thread_msg(RTCTranslationStopReq)

    pReq.contents.translation_id = translation_id
    pReq.contents.ptr = None

    pRsp = sync_msg(call_dst, RTC_TRANSLATION_STOP_REQ, pReq, RTC_TRANSLATION_STOP_RSP, RTCTranslationStopRsp)
    if pRsp == None:
        return False
    return True


def dave_call_rtc_translation_data(call_dst, translation_id, sequence_number, translation_data):
    if type(call_dst) == str:
        if thread_id(call_dst) == -1:
            return False

    if type(translation_id) == str:
        translation_id = bytes(translation_id, encoding='utf8')

    pReq = thread_msg(RTCTranslationDataReq)

    pReq.contents.translation_id = translation_id
    pReq.contents.sequence_number = sequence_number
    pReq.contents.payload_data = byte_to_mbuf(translation_data)
    pReq.contents.ptr = None

    write_msg(call_dst, RTC_TRANSLATION_DATA_REQ, pReq)
    return
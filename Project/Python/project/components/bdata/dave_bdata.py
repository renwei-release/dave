# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from public import *
import socket
import time
import uuid


BDATA_THREAD_NAME = b"bdata"
HOSTNAME = socket.gethostname().encode()


def _myline(depth):
    current_depth = len(inspect.stack())
    if depth >= current_depth:
        return 'NULL'.encode("utf-8"), 0
    __func__ = sys._getframe(depth).f_code.co_name.encode("utf-8")
    __LINE__ = sys._getframe(depth).f_lineno
    return __func__, __LINE__


def _localtime(local_date):
    time_tuple = time.localtime(time.time())

    local_date.year = time_tuple[0]
    local_date.month = time_tuple[1]
    local_date.day = time_tuple[2]
    local_date.hour = time_tuple[3]
    local_date.minute = time_tuple[4]
    local_date.second = time_tuple[5]
    return


def __BDATABASE__(sub, pReq):
    pReq.contents.version = dave_verno()
    if isinstance(sub, str):
        pReq.contents.sub_flag = sub.encode()
    else:
        pReq.contents.sub_flag = sub

    _localtime(pReq.contents.local_date)

    __func__, __LINE__ = _myline(3)
    pReq.contents.fun = __func__
    pReq.contents.line = __LINE__

    pReq.contents.host_name = HOSTNAME

    pReq.contents.ptr = None
    return


# =====================================================================


def BDATALOG(sub, *msg: object):
    msg = str(msg[0])

    pReq = thread_msg(BDataLogReq)
    pReq.contents.level = BDataLogLevel_normal
    __BDATABASE__(sub, pReq)
    pReq.contents.log_data = str_to_mbuf(msg)
    pReq.contents.log_file = None

    write_msg(BDATA_THREAD_NAME, BDATA_LOG_REQ, pReq)
    return True


def BDATARPT(sub, file_data, *msg: object):
    msg = str(msg[0])

    pReq = thread_msg(BDataLogReq)
    pReq.contents.level = BDataLogLevel_report
    __BDATABASE__(sub, pReq)
    pReq.contents.log_data = str_to_mbuf(msg)
    if file_data == None:
        pReq.contents.log_file = None
    else:
        pReq.contents.log_file = dict_to_mbuf(file_data)

    write_msg(BDATA_THREAD_NAME, BDATA_LOG_REQ, pReq)
    return True
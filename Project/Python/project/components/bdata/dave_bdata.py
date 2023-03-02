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


BDATA_THREAD_NAME=b"bdata"


def _myline(depth):
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

    pReq.contents.host_name = socket.gethostname().encode()

    pReq.contents.ptr = None
    return


# =====================================================================


def BDATALOG(sub, *msg: object):
    msg = str(msg[0])

    pReq = thread_msg(BDataLogReq)
    __BDATABASE__(sub, pReq)
    pReq.contents.log_data = str_to_mbuf(msg)

    pRsp = write_co(BDATA_THREAD_NAME, BDATA_LOG_REQ, pReq, BDATA_LOG_RSP, BDataLogRsp)
    if pRsp == None:
        return False

    if pRsp.ret != RetCode_OK:
        DAVELOG(f"ret:{t_auto_RetCode_str(pRsp.ret)} msg:{msg}")
        return False

    return True
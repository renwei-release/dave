# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from public import *


STORE_THREAD_NAME=b"store"


# =====================================================================


def STORESQL(*sql: object):
    sql = str(sql[0])

    pReq = thread_msg(StoreMysqlReq)
    pReq.contents.sql = str_to_mbuf(sql)
    pReq.contents.ptr = None

    pRsp = write_co(STORE_THREAD_NAME, STORE_MYSQL_REQ, pReq, STORE_MYSQL_RSP, StoreMysqlRsp)
    if pRsp.ret != RetCode_OK:
        DAVELOG(f"ret:{pRsp.ret}/{t_auto_RetCode_str(pRsp.ret)}, sql:{sql} rsp:{mbuf_to_dict(pRsp.data)}")
        return pRsp.ret, None

    sql_ret = mbuf_to_dict(pRsp.data)

    dave_mfree(pRsp.data)

    return RetCode_OK, sql_ret
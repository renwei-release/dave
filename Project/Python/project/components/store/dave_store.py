# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from public import *
from public.base.dave_log import DAVELOG


STORE_THREAD_NAME=b"store"


# =====================================================================


def STORESQL(*sql: object):
    sql = str(sql[0])

    pReq = thread_msg(StoreMysqlReq)
    pReq.contents.sql = str_to_mbuf(sql)
    pReq.contents.ptr = None

    pRsp = write_co(STORE_THREAD_NAME, STORE_MYSQL_REQ, pReq, STORE_MYSQL_RSP, StoreMysqlRsp)
    if pRsp == None:
        DAVELOG(f"pRsp is None / sql:{sql}")
        return ERRCODE_ptr_null, None

    if (pRsp.ret != RetCode_OK) and (pRsp.ret != RetCode_empty_data) and (pRsp.ret != RetCode_table_exist):
        DAVELOG(f"ret:{t_auto_RetCode_str(pRsp.ret)}/{pRsp.msg}, sql:{sql}")
        return pRsp.ret, None

    sql_array = mbuf_to_dict(pRsp.data)

    dave_mfree(pRsp.data)

    return RetCode_OK, sql_array


def STORELOAD(sql_array, row, column):
    if sql_array == None:
        return None
    if row >= len(sql_array):
        DAVELOG(f"sql_array:{sql_array} row:{row} is None")
        return None
    if column >= len(sql_array[row]):
        DAVELOG(f"sql_array:{sql_array} row:{row} column:{column} is None")
        return None

    return sql_array[row][column]


def STORELOADStr(sql_array, column):
    if sql_array == None:
        return None

    str_str = STORELOAD(sql_array, 0, column)
    if str_str == None:
        DAVELOG(f"sql_array:{sql_array} column:{column} is None")
        return None
    return str_str


def STORESQLCREATETABLE(db_name, table_name, table_disc):
    _, data_array = STORESQL(f"SHOW TABLES IN {db_name} LIKE '{table_name}'")
    if (data_array != None) and len(data_array) != 0:
        return
    STORESQL(f"CREATE TABLE {db_name}.{table_name} {table_disc}")
    return


def STORESQLCLEANTABLE(db_name, table_name, table_disc):
    _, data_array = STORESQL(f"SHOW TABLES IN {db_name} LIKE '{table_name}'")
    if (data_array != None) and len(data_array) != 0:
        STORESQL(f"DROP TABLE {db_name}.{table_name}")
        DAVELOG(f"STORESQLCLEANTABLE: DROP TABLE {db_name}.{table_name}")
    STORESQL(f"CREATE TABLE {db_name}.{table_name} {table_disc}")
    return
# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from public import *
from public.base.dave_log import DAVELOG
from public.tools.t_sys import t_sys_myline


STORE_THREAD_NAME=b"store"


def _store_mysql_co(sql):
    pReq = thread_msg(StoreMysqlReq)
    pReq.contents.sql = str_to_mbuf(sql)
    pReq.contents.ptr = None

    pRsp = write_co(STORE_THREAD_NAME, STORE_MYSQL_REQ, pReq, STORE_MYSQL_RSP, StoreMysqlRsp)
    return pRsp 


def _store_mysql_sync(sql):
    pReq = thread_msg(StoreMysqlReq)
    pReq.contents.sql = str_to_mbuf(sql)
    pReq.contents.ptr = None

    pRsp = sync_msg(STORE_THREAD_NAME, STORE_MYSQL_REQ, pReq, STORE_MYSQL_RSP, StoreMysqlRsp)
    return pRsp


def _store_pgsql_co(sql):
    pReq = thread_msg(StorePgsqlReq)
    pReq.contents.sql = str_to_mbuf(sql)
    pReq.contents.ptr = None

    pRsp = write_co(STORE_THREAD_NAME, STORE_PGSQL_REQ, pReq, STORE_PGSQL_RSP, StorePgsqlRsp)
    return pRsp


def _store_pgsql_sync(sql):
    pReq = thread_msg(StorePgsqlReq)
    pReq.contents.sql = str_to_mbuf(sql)
    pReq.contents.ptr = None

    pRsp = sync_msg(STORE_THREAD_NAME, STORE_PGSQL_REQ, pReq, STORE_PGSQL_RSP, StorePgsqlRsp)
    return pRsp


def _store_oracle_co(sql):
    pReq = thread_msg(StoreOracleReq)
    pReq.contents.sql = str_to_mbuf(sql)
    pReq.contents.ptr = None

    pRsp = write_co(STORE_THREAD_NAME, STORE_ORACLE_REQ, pReq, STORE_ORACLE_RSP, StoreOracleRsp)
    return pRsp


def _store_oracle_sync(sql):
    pReq = thread_msg(StoreOracleReq)
    pReq.contents.sql = str_to_mbuf(sql)
    pReq.contents.ptr = None

    pRsp = sync_msg(STORE_THREAD_NAME, STORE_ORACLE_REQ, pReq, STORE_ORACLE_RSP, StoreOracleRsp)
    return pRsp


def _store_mysql(sql, __func__, __LINE__):
    if dave_python_work_mode() == b'Coroutine Outer Loop':
        pRsp = _store_mysql_co(sql)
    else:
        pRsp = _store_mysql_sync(sql)

    if pRsp == None:
        DAVELOG(f"pRsp is None / sql:{sql}")
        return ERRCODE_ptr_null, None

    if (pRsp.ret != RetCode_OK) and (pRsp.ret != RetCode_empty_data) and (pRsp.ret != RetCode_table_exist):
        DAVELOG(f"<-<{__func__}:{__LINE__}> ret:{t_auto_RetCode_str(pRsp.ret)}/{pRsp.msg}, sql:{sql}")
        return pRsp.ret, None

    sql_array = mbuf_to_dict(pRsp.data)

    dave_mfree(pRsp.data)

    if sql_array == None:
        return RetCode_empty_data, None

    return RetCode_OK, sql_array


def _store_pgsql(sql, __func__, __LINE__):
    if dave_python_work_mode() == b'Coroutine Outer Loop':
        pRsp = _store_pgsql_co(sql)
    else:
        pRsp = _store_pgsql_sync(sql)

    if pRsp == None:
        DAVELOG(f"pRsp is None / sql:{sql}")
        return ERRCODE_ptr_null, None

    if (pRsp.ret != RetCode_OK) and (pRsp.ret != RetCode_empty_data) and (pRsp.ret != RetCode_table_exist):
        DAVELOG(f"<-<{__func__}:{__LINE__}> ret:{t_auto_RetCode_str(pRsp.ret)}/{pRsp.msg}, sql:{sql}")
        return pRsp.ret, None

    sql_array = mbuf_to_dict(pRsp.data)

    dave_mfree(pRsp.data)

    if sql_array == None:
        return RetCode_empty_data, None

    return RetCode_OK, sql_array


def _store_oracle(sql, __func__, __LINE__):
    if dave_python_work_mode() == b'Coroutine Outer Loop':
        pRsp = _store_oracle_co(sql)
    else:
        pRsp = _store_oracle_sync(sql)

    if pRsp == None:
        DAVELOG(f"pRsp is None / sql:{sql}")
        return ERRCODE_ptr_null, None

    if (pRsp.ret != RetCode_OK) and (pRsp.ret != RetCode_empty_data) and (pRsp.ret != RetCode_table_exist):
        DAVELOG(f"<-<{__func__}:{__LINE__}> ret:{t_auto_RetCode_str(pRsp.ret)}/{pRsp.msg}, sql:{sql}")
        return pRsp.ret, None

    sql_array = mbuf_to_dict(pRsp.data)

    dave_mfree(pRsp.data)

    if sql_array == None:
        return RetCode_empty_data, None

    return RetCode_OK, sql_array


__store_task__ = {}
__store_task_lock__ = threading.Lock()
__store_task_cond__ = threading.Condition(__store_task_lock__)
__STORE_PENDING__ = object()


def _store_loop(src_name, src_id, msg_len, msg_body):
    global __store_task__
    global __store_task_lock__
    global __store_task_cond__

    pLoop = struct_copy(MsgInnerLoop, msg_body, msg_len)

    params = mbuf_to_dict(pLoop.param, free_mbuf=True)
    if params == None:
        DAVEABNORMAL(f"params is None / src_name:{src_name} / src_id:{src_id}")
        return

    task_id = params.get("task_id")
    store_type = params.get("store_type")
    sql = params.get("sql")
    __func__ = params.get("__func__")
    __LINE__ = params.get("__LINE__")

    if store_type == "mysql":
        ret, sql_array = _store_mysql(sql, __func__, __LINE__)
    elif store_type == "pgsql":
        ret, sql_array = _store_pgsql(sql, __func__, __LINE__)
    elif store_type == "oracle":
        ret, sql_array = _store_oracle(sql, __func__, __LINE__)
    else:
        ret, sql_array = ERRCODE_ptr_null, None

    with __store_task_cond__:
        __store_task__[task_id] = (ret, sql_array)
        __store_task_cond__.notify_all()
    return


# =====================================================================


def STOREMYSQL(*sql: object):
    global __store_task__
    global __store_task_lock__
    global __store_task_cond__
    global __STORE_PENDING__

    sql = str(sql[0])

    with __store_task_cond__:
        task_id = t_rand()
        while task_id in __store_task__:
            task_id = t_rand()
        __store_task__[task_id] = __STORE_PENDING__

    __func__, __LINE__ = t_sys_myline(2)

    inner_loop(_store_loop, {
        "task_id": task_id,
        "store_type": "mysql",
        "sql": sql,
        "__func__": str(__func__),
        "__LINE__": str(__LINE__),
    }, SQL_LOOP_MSG)

    with __store_task_cond__:
        while __store_task__.get(task_id) is __STORE_PENDING__:
            __store_task_cond__.wait()
        store_result = __store_task__.pop(task_id, {})

    return store_result


def STOREPGSQL(*sql: object):
    global __store_task__
    global __store_task_lock__
    global __store_task_cond__
    global __STORE_PENDING__

    sql = str(sql[0])

    with __store_task_cond__:
        task_id = t_rand()
        while task_id in __store_task__:
            task_id = t_rand()
        __store_task__[task_id] = __STORE_PENDING__

    __func__, __LINE__ = t_sys_myline(2)

    inner_loop(_store_loop, {
        "task_id": task_id,
        "store_type": "pgsql",
        "sql": sql,
        "__func__": str(__func__),
        "__LINE__": str(__LINE__),
    }, SQL_LOOP_MSG)

    with __store_task_cond__:
        while __store_task__.get(task_id) is __STORE_PENDING__:
            __store_task_cond__.wait()
        store_result = __store_task__.pop(task_id, {})

    return store_result


def STOREORACLE(*sql: object):
    global __store_task__
    global __store_task_lock__
    global __store_task_cond__
    global __STORE_PENDING__

    sql = str(sql[0])

    with __store_task_cond__:
        task_id = t_rand()
        while task_id in __store_task__:
            task_id = t_rand()
        __store_task__[task_id] = __STORE_PENDING__

    __func__, __LINE__ = t_sys_myline(2)

    inner_loop(_store_loop, {
        "task_id": task_id,
        "store_type": "oracle",
        "sql": sql,
        "__func__": str(__func__),
        "__LINE__": str(__LINE__),
    }, SQL_LOOP_MSG)

    with __store_task_cond__:
        while __store_task__.get(task_id) is __STORE_PENDING__:
            __store_task_cond__.wait()
        store_result = __store_task__.pop(task_id, {})

    return store_result


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


def STOREMYSQLCREATETABLE(db_name, table_name, table_disc):
    _, data_array = STOREMYSQL(f"SHOW DATABASES LIKE '{db_name}'")
    if (data_array == None) or (len(data_array) == 0):
        STOREMYSQL(f"CREATE DATABASE {db_name}")

    _, data_array = STOREMYSQL(f"SHOW TABLES IN {db_name} LIKE '{table_name}'")
    if (data_array == None) or len(data_array) == 0:
        STOREMYSQL(f"CREATE TABLE {db_name}.{table_name} {table_disc}")
    return


def STOREMYSQLCLEANTABLE(db_name, table_name, table_disc):
    _, data_array = STOREMYSQL(f"SHOW DATABASES LIKE '{db_name}'")
    if (data_array == None) or (len(data_array) == 0):
        STOREMYSQL(f"CREATE DATABASE {db_name}")

    _, data_array = STOREMYSQL(f"SHOW TABLES IN {db_name} LIKE '{table_name}'")
    if (data_array != None) and len(data_array) != 0:
        STOREMYSQL(f"DROP TABLE {db_name}.{table_name}")
        DAVELOG(f"STOREMYSQLCLEANTABLE: DROP TABLE {db_name}.{table_name}")

    STOREMYSQL(f"CREATE TABLE {db_name}.{table_name} {table_disc}")
    return


def STOREPGSQLCREATETABLE(db_name, table_name, table_disc):
    _, data_array = STOREPGSQL(f"SHOW DATABASES LIKE '{db_name}'")
    if (data_array == None) or (len(data_array) == 0):
        STOREPGSQL(f"CREATE DATABASE {db_name}")

    _, data_array = STOREPGSQL(f"SHOW TABLES IN {db_name} LIKE '{table_name}'")
    if (data_array == None) or len(data_array) == 0:
        STOREPGSQL(f"CREATE TABLE {db_name}.{table_name} {table_disc}")
    return


def STOREPGSQLCLEANTABLE(db_name, table_name, table_disc):
    _, data_array = STOREPGSQL(f"SHOW DATABASES LIKE '{db_name}'")
    if (data_array == None) or (len(data_array) == 0):
        STOREPGSQL(f"CREATE DATABASE {db_name}")

    _, data_array = STOREPGSQL(f"SHOW TABLES IN {db_name} LIKE '{table_name}'")
    if (data_array != None) and len(data_array) != 0:
        STOREPGSQL(f"DROP TABLE {db_name}.{table_name}")
        DAVELOG(f"STOREPGSQLCLEANTABLE: DROP TABLE {db_name}.{table_name}")

    STOREPGSQL(f"CREATE TABLE {db_name}.{table_name} {table_disc}")
    return
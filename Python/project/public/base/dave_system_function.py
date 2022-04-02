# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from .dave_msg_id import *


def fun_None(src_name, src_id, msg_len, msg_body):
    return


system_function_table = {
    MSGID_TEST: fun_None,
    MSGID_TIMER: fun_None,
    MSGID_WAKEUP: fun_None,
    MSGID_RUN_FUNCTION: fun_None,
    MSGID_RESTART_REQ: fun_None,
    MSGID_RESTART_RSP: fun_None,
    MSGID_POWER_OFF: fun_None,
    MSGID_REMOTE_THREAD_READY: fun_None,
    MSGID_REMOTE_THREAD_REMOVE: fun_None,
    MSGID_TRACE_SWITCH: fun_None,
    MSGID_REMOTE_MSG_TIMER_OUT: fun_None,
    MSGID_CALL_FUNCTION: fun_None,
    MSGID_MEMORY_WARNING: fun_None,
    MSGID_ECHO: fun_None,
    MSGID_INTERNAL_EVENTS: fun_None,
    MSGID_THREAD_BUSY: fun_None,
    MSGID_THREAD_IDLE: fun_None,
    MSGID_CLIENT_BUSY: fun_None,
    MSGID_CLIENT_IDLE: fun_None,
    MSGID_LOCAL_THREAD_READY: fun_None,
    MSGID_LOCAL_THREAD_REMOVE: fun_None,
    MSGID_SYSTEM_MOUNT: fun_None,
    MSGID_SYSTEM_DECOUPLING: fun_None,
    MSGID_REMOTE_THREAD_ID_READY: fun_None,
    MSGID_REMOTE_THREAD_ID_REMOVE: fun_None,
    MSGID_CFG_UPDATE: fun_None,
}


def dave_system_function_table_add(msg_id, msg_function):
    system_function_table[msg_id] = msg_function
    return


def dave_system_function_table_del(msg_id):
    del system_function_table[msg_id]
    return

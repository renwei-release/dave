# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from public import *


def fun_MSGID_REMOTE_THREAD_ID_READY(src_name, src_id, msg_len, msg_body):
    pReady = struct_copy(ThreadRemoteIDReadyMsg, msg_body, msg_len)
    DAVELOG(f'{pReady.remote_thread_id} {pReady.remote_thread_name} {pReady.globally_identifier}')
    return


def fun_MSGID_REMOTE_THREAD_ID_REMOVE(src_name, src_id, msg_len, msg_body):
    pRemove = struct_copy(ThreadRemoteIDRemoveMsg, msg_body, msg_len)
    DAVELOG(f'{pRemove.remote_thread_id} {pRemove.remote_thread_name} {pRemove.globally_identifier}')
    return


# =====================================================================


def dave_product_init():
    dave_system_function_table_add(MSGID_REMOTE_THREAD_ID_READY, fun_MSGID_REMOTE_THREAD_ID_READY)
    dave_system_function_table_add(MSGID_REMOTE_THREAD_ID_REMOVE, fun_MSGID_REMOTE_THREAD_ID_REMOVE)
    return


def dave_product_exit():
    dave_system_function_table_del(MSGID_REMOTE_THREAD_ID_READY)
    dave_system_function_table_del(MSGID_REMOTE_THREAD_ID_REMOVE)
    return
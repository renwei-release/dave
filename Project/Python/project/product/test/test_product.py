# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import threading
from public import *
from product.test.test.unit_test.unit_test import unit_test


_service_table = {}


def service_ready_handler():
    pEvents = thread_msg(InternalEvents)
    write_msg('TEST', MSGID_INTERNAL_EVENTS, pEvents)
    return


_service_ready_timer = threading.Timer(180, service_ready_handler)


def fun_MSGID_INTERNAL_EVENTS(src_name, src_id, msg_len, msg_body):
    unit_test(_service_table)
    return


def fun_MSGID_REMOTE_THREAD_ID_READY(src_name, src_id, msg_len, msg_body):
    global _service_ready_timer

    pReady = struct_copy(ThreadRemoteIDReadyMsg, msg_body, msg_len)

    _service_table[pReady.remote_thread_name.lower()] = { pReady.remote_thread_id }

    _service_ready_timer.cancel()
    _service_ready_timer = threading.Timer(3, service_ready_handler)
    _service_ready_timer.start()
    return


# =====================================================================


def dave_product_init():
    dave_system_function_table_add(MSGID_INTERNAL_EVENTS, fun_MSGID_INTERNAL_EVENTS)
    dave_system_function_table_add(MSGID_REMOTE_THREAD_ID_READY, fun_MSGID_REMOTE_THREAD_ID_READY)
    return


def dave_product_exit():
    dave_system_function_table_del(MSGID_INTERNAL_EVENTS)
    dave_system_function_table_del(MSGID_REMOTE_THREAD_ID_READY)
    return
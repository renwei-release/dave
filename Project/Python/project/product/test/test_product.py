# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import threading
from public import *
from product.test.test.test_service.test_service import test_service


_service_table = {}


def service_test_exit():
    dave_poweroff()
    return


_service_ready_timer = threading.Timer(360, service_test_exit)


def fun_MSGID_REMOTE_THREAD_ID_READY(src_name, src_id, msg_len, msg_body):
    global _service_ready_timer

    _service_ready_timer.cancel()

    pReady = struct_copy(ThreadRemoteIDReadyMsg, msg_body, msg_len)

    test_service(pReady.globally_identifier, pReady.remote_thread_name.lower(), pReady.remote_thread_id)

    _service_ready_timer = threading.Timer(3, service_test_exit)
    _service_ready_timer.start()
    return


# =====================================================================


def dave_product_init():
    dave_system_function_table_add(MSGID_REMOTE_THREAD_ID_READY, fun_MSGID_REMOTE_THREAD_ID_READY)
    return


def dave_product_exit():
    dave_system_function_table_del(MSGID_REMOTE_THREAD_ID_READY)
    return
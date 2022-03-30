# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from ctypes import *
from .dave_dll import dave_dll
from .dave_log import *
from .dave_struct import *
from .dave_msg_struct import *
from .dave_verno import *
from .dave_define import *
from ..tools import *


davelib = dave_dll()


def thread_self():
    __func__, __LINE__ = t_sys_myline(2)
    self_id = davelib.dave_thread_get_self(c_char_p(__func__), c_int(__LINE__))
    davelib.dave_thread_get_name.restype = c_char_p
    return davelib.dave_thread_get_name(c_longlong(self_id), c_char_p(__func__), c_int(__LINE__))


def thread_msg(class_struct):
    __func__, __LINE__ = t_sys_myline(2)
    davelib.dave_thread_msg.restype = POINTER(class_struct)
    return davelib.dave_thread_msg(sizeof(class_struct), c_bool(True), c_char_p(__func__), c_int(__LINE__))


def write_msg(dst, msg_id, class_instance):
    __func__, __LINE__ = t_sys_myline(2)
    DAVEDEBUG(f"dst type:{type(dst)} data:{dst}")
    if isinstance(dst, c_char) or isinstance(dst, c_char_p) or isinstance(dst, str) or isinstance(dst, bytes):
        davelib.dave_thread_remote_msg(c_char_p(dst), c_longlong(msg_id), c_int(sizeof(class_instance.contents)), class_instance, c_char_p(__func__), c_int(__LINE__))
    else:
        davelib.dave_thread_local_msg(c_uint64(-1), c_uint64(dst), c_int(0), c_longlong(msg_id), c_uint64(sizeof(class_instance.contents)), class_instance, c_char_p(__func__), c_int(__LINE__))
    return


def broadcast_msg(thread_name, msg_id, class_instance):
    __func__, __LINE__ = t_sys_myline(2)
    davelib.dave_thread_broadcast_msg(c_int(BaseMsgType_Broadcast_thread), c_char_p(thread_name), c_longlong(msg_id), c_uint64(sizeof(class_instance.contents)), class_instance, c_char_p(__func__), c_int(__LINE__))
    return


def fun_register(thread_name, function_id):
    pRegister = thread_msg(AppMsgFunctionRegReq)
    pRegister.contents.thread_name = dave_product()
    pRegister.contents.function_id = function_id
    broadcast_msg(thread_name, APPMSG_FUNCTION_REGISTER_REQ, pRegister)
    return


def io_register(function_id):
    fun_register(AIB_THREAD_NAME, function_id)
    return


def cv_register(function_id):
    fun_register(CV_THREAD_NAME, function_id)
    return
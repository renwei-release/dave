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
from .dave_verno import *
from ..tools import *


davelib = dave_dll()


def thread_msg(class_struct):
    __func__, __LINE__ = t_sys_myline(2)
    davelib.dave_dll_thread_msg.restype = POINTER(class_struct)
    return davelib.dave_dll_thread_msg(sizeof(class_struct), c_char_p(__func__), c_int(__LINE__))


def write_msg(dst, msg_id, class_instance):
    __func__, __LINE__ = t_sys_myline(2)
    DAVEDEBUG(f"dst type:{type(dst)} data:{dst}")
    if isinstance(dst, c_char) or isinstance(dst, c_char_p) or isinstance(dst, str) or isinstance(dst, bytes):
        davelib.dave_dll_thread_name_msg(c_char_p(dst), c_longlong(msg_id), c_int(sizeof(class_instance.contents)), class_instance, c_char_p(__func__), c_int(__LINE__))
    else:
        davelib.dave_dll_thread_id_msg(c_uint64(-1), c_uint64(dst), c_int(0), c_longlong(msg_id), c_uint64(sizeof(class_instance.contents)), class_instance, c_char_p(__func__), c_int(__LINE__))
    return


def broadcast_msg(thread_name, msg_id, class_instance):
    __func__, __LINE__ = t_sys_myline(2)
    davelib.dave_dll_thread_broadcast_msg(c_char_p(thread_name), c_longlong(msg_id), c_uint64(sizeof(class_instance.contents)), class_instance, c_char_p(__func__), c_int(__LINE__))
    return
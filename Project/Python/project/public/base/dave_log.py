# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import string
import sys
import inspect
from ctypes import *
from .dave_dll import dave_dll


davelib = dave_dll()


def _myline(depth):
    current_depth = len(inspect.stack())
    if depth >= current_depth:
        return 'NULL'.encode("utf-8"), 0
    __func__ = sys._getframe(depth).f_code.co_name.encode("utf-8")
    __LINE__ = sys._getframe(depth).f_lineno
    return __func__, __LINE__


def __dave_log__(*log_msg: object, log_type):
    __func__, __LINE__ = _myline(3)
    log_msg = str(log_msg[0][0]).encode("utf-8")
    davelib.dave_dll_log(c_char_p(__func__), c_int(__LINE__), c_char_p(log_msg), c_int(log_type))
    return


# =====================================================================


def DAVEDEBUG(*log_msg: object):
    # __dave_log__(log_msg, log_type=0)
    return


def DAVETRACE(*log_msg: object):
    __dave_log__(log_msg, log_type=1)
    return


def DAVELOG(*log_msg: object):
    __dave_log__(log_msg, log_type=2)
    return


def DAVEABNORMAL(*log_msg: object):
    __dave_log__(log_msg, log_type=3)
    return
# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from ctypes import *
from string import *
from .dave_dll import *


davelib=dave_dll()


# =====================================================================


TIMERREGFUNC=CFUNCTYPE(None, c_longlong, c_longlong)
def dave_timer_creat(name, alarm_second, timer_fun):
    name = bytes(name, encoding="utf8")
    davelib.dave_dll_timer_creat.restype = c_int
    ret = davelib.dave_dll_timer_creat(c_char_p(name), c_int(alarm_second), timer_fun)
    if ret < 0:
        return False
    return True


def dave_timer_kill(name):
    davelib.dave_dll_timer_kill(c_char_p(name))
    return
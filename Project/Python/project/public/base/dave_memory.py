# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import string
import sys
import inspect
from ctypes import *
from .dave_dll import dave_dll
from .dave_tools import *
from ..auto import *
from ..tools import *


davelib=dave_dll()


# =====================================================================


def dave_mmalloc(len):
    __func__, __LINE__ = t_sys_myline(2)
    davelib.dave_dll_mmalloc.restype = POINTER(MBUF)
    return davelib.dave_dll_mmalloc(c_int(len), c_char_p(__func__), c_int(__LINE__))


def dave_mfree(ptr):
    __func__, __LINE__ = t_sys_myline(2)
    davelib.dave_dll_mfree(ptr, c_char_p(__func__), c_int(__LINE__))
    return


def dave_mclone(ptr):
    __func__, __LINE__ = t_sys_myline(2)
    davelib.dave_dll_mclone.restype = POINTER(MBUF)
    return davelib.dave_dll_mclone(ptr, c_char_p(__func__), c_int(__LINE__))
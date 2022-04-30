# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from ctypes import *
from .dave_dll import dave_dll

davelib = dave_dll()


def cfg_set(key, value):
    key = bytes(key, encoding="utf8")
    value_len = len(value)
    value = bytes(value, encoding="utf8")
    davelib.dave_dll_cfg_set(c_char_p(key), c_char_p(value), c_int(value_len))
    return


def cfg_get(key, default_value=None):
    key = bytes(key, encoding="utf8")
    value = bytes(2048)
    davelib.dave_dll_cfg_get.restype = c_int
    ret = davelib.dave_dll_cfg_get(c_char_p(key), c_char_p(value), c_int(len(value)))
    if ret < 0:
        return default_value
    value = str(value, encoding="utf8").replace("\0", "")
    return value
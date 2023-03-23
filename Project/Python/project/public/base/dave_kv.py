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
import json
from ctypes import *
from .dave_dll import dave_dll
from .dave_log import *


davelib=dave_dll()


TIMEROUTFUNC=CFUNCTYPE(None, c_void_p, c_char_p)
def _kv_timerout(kv, key):
    if isinstance(key, str) == True:
        key = bytes(key, encoding='utf8')

    DAVELOG(f"key:{key}")
    davelib.dave_dll_kv_del(c_void_p(kv), c_char_p(key))
    return
_c_kv_timerout = TIMEROUTFUNC(_kv_timerout)


def _kv_malloc(name, out_second):
    if isinstance(name, str) == True:
        name = bytes(name, encoding='utf8')
    davelib.dave_dll_kv_malloc.restype = c_void_p
    kv = davelib.dave_dll_kv_malloc(c_char_p(name), c_int(out_second), _c_kv_timerout)
    return kv


def _kv_free(kv):
    davelib.dave_dll_kv_free(c_void_p(kv))
    return


_default_kv = None


def _kv_init(kv):
    global _default_kv

    if kv != None:
        return kv

    if _default_kv == None:
        _default_kv = _kv_malloc("pydefault", 360)
    return _default_kv


# =====================================================================


def kv_malloc(name, out_second):
    return _kv_malloc(name, out_second)


def kv_free(kv):
    _kv_free(kv)
    return


def kv_add(key, value, kv=None):
    kv = _kv_init(kv)

    if isinstance(key, str) == True:
        key = bytes(key, encoding='utf8')
    if isinstance(value, str) == True:
        value = bytes(value, encoding='utf8')
    davelib.dave_dll_kv_add.restype = c_int
    ret = davelib.dave_dll_kv_add(c_void_p(kv), c_char_p(key), c_char_p(value))
    if ret == 0:
        return True
    return False


def kv_inq(key, kv=None):
    kv = _kv_init(kv)

    if isinstance(key, str) == True:
        key = bytes(key, encoding='utf8')

    value = bytes(1024 * 1024)

    davelib.dave_dll_kv_inq.restype = c_int
    ret = davelib.dave_dll_kv_inq(c_void_p(kv), c_char_p(key), c_char_p(value), c_int(len(value)))
    if ret <= 0:
        return None
    return value[:ret]


def kv_del(key, kv=None):
    kv = _kv_init(kv)

    if isinstance(key, str) == True:
        key = bytes(key, encoding='utf8')

    davelib.dave_dll_kv_del.restype = c_int
    davelib.dave_dll_kv_del(c_void_p(kv), c_char_p(key))
    return


def kv_add_dict(key, dict, kv=None):
    value = json.dumps(dict, ensure_ascii=False)
    return kv_add(key, value, kv)


def kv_inq_dict(key, kv=None):
    value = kv_inq(key, kv)
    if value == None:
        return None
    return json.loads(value)


def kv_del_dict(key, kv=None):
    return kv_del(key, kv)
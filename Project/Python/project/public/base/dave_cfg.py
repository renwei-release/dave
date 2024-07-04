# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from ctypes import *
from .dave_dll import dave_dll
import json


davelib = dave_dll()


# =====================================================================


def cfg_set(key, value):
    key = bytes(key, encoding="utf8")
    value = bytes(value, encoding="utf8")
    davelib.dave_dll_cfg_set(c_char_p(key), c_char_p(value))
    return


def cfg_get(key, default_value=None):
    byte_key = bytes(key, encoding="utf8")
    value = bytes(2048)
    davelib.dave_dll_cfg_get.restype = c_int
    ret = davelib.dave_dll_cfg_get(c_char_p(byte_key), c_char_p(value), c_int(len(value)))
    if ret < 0:
        if default_value != None:
            cfg_set(key, default_value)
        return default_value
    value = str(value, encoding="utf8").replace("\0", "")
    return value


def cfg_del(key):
    byte_key = bytes(key, encoding="utf8")
    davelib.dave_dll_cfg_del(c_char_p(byte_key))
    return


def cfg_set_dict(key, value_dict):
    value_str = json.dumps(value_dict)
    cfg_set(key, value_str)
    return


def cfg_get_dict(key, default_value=None):
    value_str = cfg_get(key, json.dumps(default_value))
    try:
        value_dict = json.loads(value_str)
    except Exception as e:
        value_dict = default_value
        cfg_set_dict(key, value_dict)
    return value_dict


def cfg_del_dict(key):
    cfg_del(key)
    return


def cfg_get_float(key, default_value=None):
    default_value_string = str(default_value)
    value = cfg_get(key, default_value_string)
    if value == default_value_string:
        return default_value
    try:
        if value.find(".") < 0:
            data_value = int(value)
        else:
            data_value = float(value)
    except Exception as e:
        print(f"cfg_get_float error:{e} key:{key} value:{value}")
        data_value = default_value
    return data_value


def cfg_get_ub(key, default_value=None):
    return cfg_get_float(key, default_value)


def cfg_get_bool(key, default_value=False):
    if default_value == True:
        default_value_string = "true"
    else:
        default_value_string = "false"
    value = cfg_get(key, default_value_string)
    if value == "true":
        return True
    else:
        return False


CFGREGFUNC=CFUNCTYPE(None, c_char_p, c_int, c_char_p, c_int)
def cfg_reg(key, reg_fun):
    byte_key = bytes(key, encoding="utf8")
    davelib.dave_dll_cfg_reg.restype = c_int
    davelib.dave_dll_cfg_reg(c_char_p(byte_key), reg_fun)
    return


def rcfg_set(key, value, ttl=0):
    key = bytes(key, encoding="utf8")
    value = bytes(value, encoding="utf8")
    davelib.dave_dll_cfg_remote_set(c_char_p(key), c_char_p(value), c_int(ttl))
    return


def rcfg_get(key, default_value=None):
    key = bytes(key, encoding="utf8")
    value = bytes(1024 * 10)
    davelib.dave_dll_cfg_get.restype = c_int
    ret = davelib.dave_dll_cfg_remote_get(c_char_p(key), c_char_p(value), c_int(len(value)))
    if ret < 0:
        return default_value
    value = str(value, encoding="utf8").replace("\0", "")
    return value


def rcfg_del(key):
    key = bytes(key, encoding="utf8")
    davelib.dave_dll_cfg_remote_del(c_char_p(key))
    return
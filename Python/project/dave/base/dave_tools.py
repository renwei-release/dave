# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import json
from ctypes import *
from string import *
from .dave_log import *
from .dave_memory import *


davelib=dave_dll()


def struct_copy(class_struct, msg_body, msg_len=0):
    if msg_len != 0 and sizeof(class_struct) != msg_len:
        DAVELOG("Wrong structure definition!!! {}:{}/{}".format(class_struct, sizeof(class_struct), msg_len))
    pStruct = class_struct()
    memmove(addressof(pStruct), msg_body, sizeof(class_struct))
    return pStruct


def mbuf_to_byte(mbuf_data):
    if mbuf_data == None:
        return None
    try:
        hasattr(mbuf_data, "contents")
    except:
        return None
    ByteArr = c_char * mbuf_data.contents.len
    byte_arr = ByteArr(*mbuf_data.contents.payload[:mbuf_data.contents.len])
    return byte_arr.raw


def byte_to_mbuf(byte_data):
    if isinstance(byte_data, str):
        byte_data = byte_data.encode("utf-8")
    mbuf_data = dave_mmalloc(len(byte_data))
    memmove(mbuf_data.contents.payload, byte_data, len(byte_data))
    return mbuf_data


def dict_to_mbuf(dict_object):
    json_str = json.dumps(dict_object, indent=4)
    return byte_to_mbuf(json_str)
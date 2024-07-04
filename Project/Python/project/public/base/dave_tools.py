# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import json
from ctypes import *
from ctypes import POINTER, c_void_p, cast
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
    if (mbuf_data is None) or (cast(mbuf_data, c_void_p).value == None):
        return None

    try:
        ByteArr = c_char * mbuf_data.contents.len
        byte_arr = ByteArr(*mbuf_data.contents.payload[:mbuf_data.contents.len])
        return byte_arr.raw
    except:
        return None

    return None


def byte_to_mbuf(byte_data):
    if (byte_data == None) or (len(byte_data) == 0):
        return None

    # Convert list of integers to bytes
    if isinstance(byte_data, list):
        byte_data = bytes(byte_data)
    # Convert string to bytes
    elif isinstance(byte_data, str):
        byte_data = byte_data.encode("utf-8")
    # For bytes or bytearray, no conversion is needed
    elif isinstance(byte_data, (bytes, bytearray)):
        pass
    else:
        raise ValueError("byte_data must be a list, str, bytes, or bytearray")

    mbuf_data = dave_mmalloc(len(byte_data))
    memmove(mbuf_data.contents.payload, byte_data, len(byte_data))
    return mbuf_data


def mbuf_to_str(mbuf_data):
    if (mbuf_data is None) or (cast(mbuf_data, c_void_p).value == None):
        return None

    byte_data = mbuf_to_byte(mbuf_data)
    if byte_data is None:
        return None
    return byte_data.decode()


def str_to_mbuf(str_data):
    return byte_to_mbuf(str_data)


def dict_to_mbuf(dict_object):
    if dict_object == None:
        return None
    json_str = json.dumps(dict_object, indent=4)
    return byte_to_mbuf(json_str)


def mbuf_to_dict(mbuf_data):
    if mbuf_data == None:
        return None
    try:
        hasattr(mbuf_data, "contents")
    except:
        return None

    str_data = mbuf_to_str(mbuf_data)
    json_dict = json.loads(str_data, strict=False)
    if json_dict == None:
        return None
    if isinstance(json_dict, str) == True:
        DAVELOG(f"json.loads error:{json_dict} / {str_data}")
        return {}
    return json_dict


def obj_to_obj(dst_obj, src_obj):
    dst_obj_name_array = dir(dst_obj)
    src_obj_name_array = dir(src_obj)

    for obj_name in dst_obj_name_array:
        if (obj_name[0] != '_') and (obj_name in src_obj_name_array):
            setattr(dst_obj, obj_name, getattr(src_obj, obj_name))
    return
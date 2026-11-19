# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import json
import base64
from ctypes import *
from .dave_dll import dave_dll
from .dave_log import *


davelib=dave_dll()


TIMEROUTFUNC=CFUNCTYPE(None, c_void_p, c_char_p)
def _kv_timerout(kv, key):
    if isinstance(key, str) == True:
        key = bytes(key, encoding='utf8')

    DAVEDEBUG(f"key:{key}")
    davelib.dave_dll_kv_del(c_void_p(kv), c_char_p(key))
    return
_c_kv_timerout = TIMEROUTFUNC(_kv_timerout)


def _kv_malloc(name, out_second):
    if isinstance(name, str) == True:
        name = bytes(name, encoding='utf8')
    davelib.dave_dll_kv_malloc.restype = c_void_p
    kv = davelib.dave_dll_kv_malloc(c_char_p(name), c_int(out_second), _c_kv_timerout)
    return kv


def _kv_remote_malloc(name, out_second):
    if isinstance(name, str) == True:
        name = bytes(name, encoding='utf8')
    davelib.dave_dll_kv_remote_malloc.restype = c_void_p
    kv = davelib.dave_dll_kv_remote_malloc(c_char_p(name), c_int(out_second), _c_kv_timerout)
    return kv


def _kv_free(kv):
    if kv == None:
        return
    davelib.dave_dll_kv_free(c_void_p(kv))
    return


_default_kv = None
def _kv_init(kv):
    global _default_kv

    if kv != None:
        return kv

    if _default_kv == None:
        _default_kv = _kv_malloc("pydefault", 3600)
    return _default_kv


def _kv_exit(kv):
    global _default_kv

    if kv == None:
        kv = _default_kv

    if kv == _default_kv:
        _default_kv = None
    _kv_free(kv)
    return


KV_MULTIPART_RAW_SIZE = 2 * 1024 * 1024
KV_MULTIPART_PREFIX = b"__DAVE_KV_MP1__:"


def _kv_to_bytes(data):
    if isinstance(data, bytes):
        return data
    if isinstance(data, str):
        return bytes(data, encoding='utf8')
    return bytes(str(data), encoding='utf8')


def _kv_part_key(key_bytes, part_index):
    return key_bytes + b"::mp::" + bytes(str(part_index), encoding='ascii')


def _kv_add_raw(kv, key_bytes, value_bytes):
    davelib.dave_dll_kv_add.restype = c_int
    ret = davelib.dave_dll_kv_add(c_void_p(kv), c_char_p(key_bytes), c_char_p(value_bytes))
    if ret == 0:
        return True
    return False


def _kv_inq_raw(kv, key_bytes):
    davelib.dave_dll_kv_inq.restype = c_int

    buf_size = KV_MULTIPART_RAW_SIZE * 2
    max_buf_size = 2 * 1024 * 1024 * 1024

    while buf_size <= max_buf_size:
        value = create_string_buffer(buf_size)
        ret = davelib.dave_dll_kv_inq(c_void_p(kv), c_char_p(key_bytes), value, c_int(buf_size))
        if ret <= 0:
            return None

        if ret < buf_size:
            return value.raw[:ret]

        # If returned length equals current buffer size, grow and retry.
        buf_size = buf_size * 2

    DAVELOG(f"kv_inq key:{key_bytes} value too large, exceeds max buffer:{max_buf_size}")
    return None


# =====================================================================


def kv_malloc(name, out_second):
    return _kv_malloc(name, out_second)


def kv_remote_malloc(name, out_second):
    return _kv_remote_malloc(name, out_second)


def kv_free(kv):
    _kv_free(kv)
    return


def kv_add(key, value, kv=None):
    kv = _kv_init(kv)

    key = _kv_to_bytes(key)
    value = _kv_to_bytes(value)

    if len(value) <= KV_MULTIPART_RAW_SIZE:
        return _kv_add_raw(kv, key, value)

    part_count = (len(value) + KV_MULTIPART_RAW_SIZE - 1) // KV_MULTIPART_RAW_SIZE
    for part_index in range(part_count):
        start = part_index * KV_MULTIPART_RAW_SIZE
        end = start + KV_MULTIPART_RAW_SIZE
        part_raw = value[start:end]
        part_b64 = base64.b64encode(part_raw)
        if _kv_add_raw(kv, _kv_part_key(key, part_index), part_b64) == False:
            for clean_index in range(part_index):
                davelib.dave_dll_kv_del(c_void_p(kv), c_char_p(_kv_part_key(key, clean_index)))
            return False

    manifest = {
        "multipart": True,
        "encoding": "base64",
        "parts": part_count,
    }
    manifest_bytes = KV_MULTIPART_PREFIX + bytes(json.dumps(manifest, separators=(",", ":")), encoding='utf8')
    if _kv_add_raw(kv, key, manifest_bytes) == False:
        for clean_index in range(part_count):
            davelib.dave_dll_kv_del(c_void_p(kv), c_char_p(_kv_part_key(key, clean_index)))
        return False

    return True


def kv_inq(key, kv=None):
    kv = _kv_init(kv)

    key = _kv_to_bytes(key)

    value = _kv_inq_raw(kv, key)
    if value == None:
        return None

    if value.startswith(KV_MULTIPART_PREFIX) == False:
        return value

    try:
        meta = json.loads(value[len(KV_MULTIPART_PREFIX):].decode('utf8'))
    except Exception as e:
        DAVELOG(f"kv_inq key:{key} multipart meta decode failed:{e}")
        return None

    if meta.get("multipart") != True or meta.get("encoding") != "base64":
        DAVELOG(f"kv_inq key:{key} multipart meta invalid")
        return None

    parts = meta.get("parts")
    if isinstance(parts, int) == False or parts <= 0:
        DAVELOG(f"kv_inq key:{key} multipart parts invalid:{parts}")
        return None

    merged = bytearray()
    for part_index in range(parts):
        part_value = _kv_inq_raw(kv, _kv_part_key(key, part_index))
        if part_value == None:
            DAVELOG(f"kv_inq key:{key} multipart part missing:{part_index}")
            return None
        try:
            merged.extend(base64.b64decode(part_value, validate=True))
        except Exception as e:
            DAVELOG(f"kv_inq key:{key} multipart base64 decode failed at part:{part_index}, error:{e}")
            return None

    return bytes(merged)


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
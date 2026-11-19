# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2026 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import json
import threading
from public.auto import *
from public.base.dave_parameter import *
from public.base.dave_verno import dave_PROJECT
from public.base.dave_log import *


BALANCED_KEY_NAME = 'key'
BALANCED_COUNTER_NAME = 'counter'
BALANCED_STAY_NAME = 'stay'
BALANCED_MAX_KEY_COUNT = 6


_thread_gid_table_lock = threading.Lock()
_thread_gid_table = {
    dave_PROJECT() : {
        globally_identifier() : {
            BALANCED_KEY_NAME : [],
            BALANCED_COUNTER_NAME : 0,
            BALANCED_STAY_NAME : 0,
        }
    }
}


def _gid_balanced_ready_msg(thread_name : str, pReady : ThreadRemoteIDReadyMsg):
    global _thread_gid_table

    if pReady.remote_thread_name != bytes(thread_name, 'utf-8'):
        return []

    if thread_name not in _thread_gid_table:
        _thread_gid_table[thread_name] = {}

    gid = pReady.globally_identifier.decode('utf-8')

    balanced_data = _thread_gid_table[thread_name]

    if gid not in balanced_data:
        _thread_gid_table[thread_name][gid] = {
            BALANCED_KEY_NAME : [],
            BALANCED_COUNTER_NAME : 0,
            BALANCED_STAY_NAME : 0
        }
        _thread_gid_table[thread_name] = dict(sorted(balanced_data.items()))

    return _thread_gid_table[thread_name]


def _gid_balanced_remove_msg(thread_name : str, pRemove : ThreadRemoteIDRemoveMsg):
    global _thread_gid_table

    if pRemove.remote_thread_name != bytes(thread_name, 'utf-8'):
        return []

    if thread_name not in _thread_gid_table:
        return []

    gid = pRemove.globally_identifier.decode('utf-8')

    balanced_data = _thread_gid_table[thread_name]

    if gid in balanced_data:
        del _thread_gid_table[thread_name][gid]

    return _thread_gid_table[thread_name]


def _gid_balanced(thread_name : str, balancing_key : str):
    global _thread_gid_table

    if thread_name not in _thread_gid_table:
        return None

    gid_list = list(_thread_gid_table[thread_name].keys())
    gid_count = len(gid_list)
    if gid_count == 0:
        return None

    collection_sum = sum(ord(c) for c in balancing_key[-gid_count:])
    gid_index = collection_sum % gid_count
    gid = gid_list[gid_index]

    balanced_data = _thread_gid_table[thread_name][gid]

    if gid == globally_identifier():
        balanced_data[BALANCED_STAY_NAME] += 1
        return None

    balanced_data[BALANCED_COUNTER_NAME] += 1
    keys = balanced_data[BALANCED_KEY_NAME]
    if balancing_key in keys:
        keys.remove(balancing_key)
    keys.append(balancing_key)
    if len(keys) > BALANCED_MAX_KEY_COUNT:
        del keys[:-BALANCED_MAX_KEY_COUNT]

    return gid


# =====================================================================


def gid_balanced_ready_msg(thread_name : str, pReady : ThreadRemoteIDReadyMsg):
    global _thread_gid_table_lock

    with _thread_gid_table_lock:
        return _gid_balanced_ready_msg(thread_name, pReady)


def gid_balanced_remove_msg(thread_name : str, pRemove : ThreadRemoteIDRemoveMsg):
    global _thread_gid_table_lock

    with _thread_gid_table_lock:
        return _gid_balanced_remove_msg(thread_name, pRemove)


def gid_balanced(thread_name : str, balancing_key : str):
    global _thread_gid_table_lock

    if thread_name is None or thread_name == '':
        return None

    if balancing_key is None or balancing_key == '':
        return None

    with _thread_gid_table_lock:
        return _gid_balanced(thread_name, balancing_key)


def gid_balanced_info():
    global _thread_gid_table_lock

    with _thread_gid_table_lock:
        return "gid balanced:\n" + json.dumps(_thread_gid_table, ensure_ascii=False, indent=4) + "\n"
# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import hashlib
import os
from autocode_cfg import *


def _load_file_hash(file_list):
    modify_time = ""
    for file_name in file_list:
        modify_time += str(os.path.getmtime(file_name))
    return hashlib.sha256(modify_time.encode()).hexdigest()


def _get_hash_file():
    try:
        with open(rpc_hash_file, "r", encoding="utf-8") as file_id:
            return file_id.read()
    except:
        return ""


def _set_hash_file(hash):
    with open(rpc_hash_file, "w+", encoding="utf-8") as file_id:
        file_id.write(f"{hash}")


def _check_file_hash(file_list):
    file_hash = _load_file_hash(file_list)
    hash_file = _get_hash_file()
    if file_hash != hash_file:
        _set_hash_file(file_hash)
        return True
    print("The header file has not changed and there is no need to rebuild the automation code!")
    return False


# =====================================================================


def check_file_hash(file_list):
    return _check_file_hash(file_list)


def clean_file_hash():
    _set_hash_file("")
    print("Set the hash file to empty!")
    return
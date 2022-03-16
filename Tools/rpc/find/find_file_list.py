# -*- coding: utf-8 -*-

#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.03.11.
#


import re
import os
from rpc_cfg import *


def __is_disable_path(current_dir):
    for disable_path in rpc_disable_path_list:
        if disable_path == current_dir:
            return True
    return False


def __find_file_list(dir_path:str, file_list:list, rules=None):
    if dir_path.endswith('/'):
        dir_path = dir_path[:-1]

    current_file_or_dir_list = os.listdir(dir_path)
    for current_file_or_dir in current_file_or_dir_list:
        current_file_or_dir = dir_path + '/' + current_file_or_dir
        if os.path.isdir(current_file_or_dir):
            if __is_disable_path(current_file_or_dir):
                continue
            __find_file_list(current_file_or_dir, file_list, rules)
        else:
            if rules != None:
                result = re.findall(rules, current_file_or_dir)
                if result:
                    file_list.append(current_file_or_dir)
            else:
                file_list.append(current_file_or_dir)


def _find_file_list(path_list, file_list:list, rules=None):
    for dir_path in path_list:
        __find_file_list(dir_path, file_list, rules)


# =====================================================================


def find_file_list():
    file_list = []
    _find_file_list(rpc_detected_path_list, file_list, ".*\.h")
    return file_list
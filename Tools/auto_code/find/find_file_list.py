# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import re
import os
import traceback
from autocode_cfg import *


def __is_disable_path(current_dir):
    for disable_path in forgettable_path_list:
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


def _find_file_list(file_path_list, file_list:list, rules=None):
    for dir_path in file_path_list:
        __find_file_list(dir_path, file_list, rules)


def _inc_file_name_to_path(inc_name_array, file_list):
    inc_path_array = []
    for inc_name in inc_name_array:
        for file_name in file_list:
            if inc_name in file_name:
                inc_path_array.append(file_name)
    return inc_path_array


def _remove_invalid_file(file_list):
    new_file_list = []
    inc_file_list = []
    for file_name in file_list:
        with open(file_name, "r", encoding="utf-8") as file_id:
            try:
                file_content = file_id.read()
                find_msg_struct = re.findall("\/\* for *(.+?) *?message *\*\/.*?", file_content)
                find_inc_file = re.findall("#include .*?[\",<](.+?.h)[\",>]", file_content)
                if len(find_msg_struct) > 0:
                    new_file_list.append(file_name)
                if len(find_inc_file) > 0:
                    find_inc_file = _inc_file_name_to_path(find_inc_file, file_list)
                    inc_file_list.extend(find_inc_file)
            except:
                print(f'file_name:{file_name} read failed!')

    for Loop_multiple_times_to_get_nested_files in range(3):
        inc_file_list = list(set(inc_file_list))
        new_inc_file_list = []
        for inc_file in inc_file_list:
            with open(inc_file, "r", encoding="utf-8") as file_id:
                try:
                    file_content = file_id.read()
                    find_inc_file = re.findall("#include .*?[\",<](.+?.h)[\",>]", file_content)
                    if len(find_inc_file) > 0:
                        find_inc_file = _inc_file_name_to_path(find_inc_file, file_list)
                        new_inc_file_list.extend(find_inc_file)
                except:
                    print(f'file_name:{file_name} read failed!')
        inc_file_list.extend(new_inc_file_list)

    new_file_list.extend(inc_file_list)
    new_file_list = list(set(new_file_list))

    return new_file_list


# =====================================================================


def find_file_list():

    file_list = []
    _find_file_list(project_path_list, file_list, ".*\.h")
    return _remove_invalid_file(file_list)
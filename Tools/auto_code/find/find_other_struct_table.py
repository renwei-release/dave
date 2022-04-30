# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import re
import traceback
from .find_file_list import find_file_list
from autocode_tools import *


def _find_struct_list_from_file(struct_list, file_name):
    valid_document = False
    with open(file_name, "r", encoding="utf-8") as file_id:
        try:
            file_content = file_id.read()
        except:
            print(f"1 _find_struct_list_from_file file_name:{file_name}")
            return valid_document
        try:
            result = get_struct_list(file_content)
            if result:
                valid_document = True
                struct_list.extend(result)
        except:
            print(f"2 _find_struct_list_from_file file_name:{file_name}")
            return valid_document      
    return valid_document


def _find_struct_list_from_file_list(file_list):
    struct_list = []
    include_list = []
    for file_name in file_list:
        valid_document = _find_struct_list_from_file(struct_list, file_name)
        if valid_document == True:
            include_list.append(file_name)
    return struct_list, include_list


def _find_struct_data_to_table(other_struct_table, struct_data):
    struct_name, base_array = get_struct_data(struct_data)
    if struct_name != None:
        other_struct_table[struct_name] = struct_table_set(None, base_array)
    return


def _find_struct_list_to_table(struct_list):
    other_struct_table = {}
    for struct_data in struct_list:
        _find_struct_data_to_table(other_struct_table, struct_data)
    return other_struct_table


def _find_remove_the_same_name_on_table(msg_struct_table, other_struct_table):
    for key in msg_struct_table.keys():
        if other_struct_table.get(key, None) != None:
            del other_struct_table[key]
    return


def _find_other_struct_use_in_msg_struct(msg_struct_table, other_struct_table):
    use_struct_table = {}
    for struct_name in msg_struct_table.keys():
        for msg_member in struct_table_get(msg_struct_table[struct_name]):
            struct_name = msg_member['t']
            struct_value = other_struct_table.get(struct_name, None)
            if struct_value != None:
                use_struct_table[struct_name] = struct_value

    new_struct_table = {}
    tmp_struct_table = use_struct_table.copy()
    for Loop_multiple_times_to_get_nested_structures in range(6):
        detected_new_struct = []
        for struct_name in tmp_struct_table.keys():
            for msg_member in struct_table_get(tmp_struct_table[struct_name]):
                struct_name = msg_member['t']
                if tmp_struct_table.get(struct_name, None) == None:
                    if other_struct_table.get(struct_name, None) != None:
                        detected_new_struct.append(struct_name)

        for struct_name in detected_new_struct:
            tmp_struct_table[struct_name] = other_struct_table[struct_name]
            new_struct_table[struct_name] = other_struct_table[struct_name]

    new_struct_table.update(use_struct_table)
    return new_struct_table


def _rebuild_include_list(include_list, other_struct_table):
    new_include_list = []
    for file_name in include_list:
        file_list = [ file_name ]
        struct_list, _ = _find_struct_list_from_file_list(file_list)
        struct_table = _find_struct_list_to_table(struct_list)
        for struct_name in struct_table.keys():
            if struct_on_the_table(struct_name, other_struct_table) == True:
                new_include_list.append(file_name)
                break
    return new_include_list


# =====================================================================


def find_other_struct_table(file_list, msg_struct_table):
    other_struct_list, include_list = _find_struct_list_from_file_list(file_list)
    other_struct_table = _find_struct_list_to_table(other_struct_list)

    _find_remove_the_same_name_on_table(msg_struct_table, other_struct_table)
    other_struct_table = _find_other_struct_use_in_msg_struct(msg_struct_table, other_struct_table)

    include_list = _rebuild_include_list(include_list, other_struct_table)

    return other_struct_table, include_list
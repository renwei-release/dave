# -*- coding: utf-8 -*-

#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.03.11.
#

import re
import traceback
from .find_file_list import find_file_list
from .find_msg_struct_table import find_msg_struct_table
from rpc_tools import *


def _find_struct_list_from_file(struct_list, file_name):
    valid_document = False
    with open(file_name, "r", encoding="utf-8") as file_id:
        try:
            file_content = file_id.read()
        except:
            print(f"file_name:{file_name}")
            traceback.print_exc()
            return valid_document
        try:
            file_content = remove_annotation_data(file_content)
            result = re.findall("typedef struct.*?\{.*?\}.*?;", file_content)
            if result:
                valid_document = True
                struct_list.extend(result)
        except:
            print(f"file_name:{file_name}")
            traceback.print_exc()
            return valid_document      
    return valid_document


def _find_struct_list_from_file_list(file_list):
    struct_list = []
    valid_file = []
    for file_name in file_list:
        valid_document = _find_struct_list_from_file(struct_list, file_name)
        if valid_document == True:
            valid_file.append(file_name)
    return struct_list, valid_file


def _find_struct_data_to_table(other_struct_table, struct_data):
    struct_name, base_array = get_struct_data(struct_data)
    if struct_name != None:
        other_struct_table[struct_name] = base_array
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
    msg_type_list = {}
    for struct_key in msg_struct_table:
        msg_struct = msg_struct_table[struct_key]
        for msg_member in msg_struct:
            msg_type_list[msg_member['t']] = msg_member['t']

    new_other_struct_table = {}
    for key in other_struct_table.keys():
        if msg_type_list.get(key, None) != None:
            new_other_struct_table[key] = other_struct_table[key]

    for Loop_multiple_times_to_get_nested_structures in range(6):
        msg_type_list = {}
        for struct_key in new_other_struct_table:
            msg_struct = new_other_struct_table[struct_key]
            for msg_member in msg_struct:
                msg_type_list[msg_member['t']] = msg_member['t']

        for key in other_struct_table.keys():
            if msg_type_list.get(key, None) != None:
                new_other_struct_table[key] = other_struct_table[key]

    return new_other_struct_table


# =====================================================================


def find_other_struct_table():
    file_list = find_file_list()
    other_struct_list, valid_struct_file = _find_struct_list_from_file_list(file_list)
    other_struct_table = _find_struct_list_to_table(other_struct_list)

    _, msg_struct_table, _ = find_msg_struct_table()
    _find_remove_the_same_name_on_table(msg_struct_table, other_struct_table)
    other_struct_table = _find_other_struct_use_in_msg_struct(msg_struct_table, other_struct_table)

    return other_struct_table, valid_struct_file
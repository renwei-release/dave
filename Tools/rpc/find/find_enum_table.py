# -*- coding: utf-8 -*-

#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.03.11.
#

import re
import traceback
from collections import Counter
from .find_file_list import *
from .find_msg_struct_table import find_msg_struct_table
from .find_other_struct_table import find_other_struct_table
from rpc_tools import *


def _find_enum_list_from_file(enum_list, file_name):
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
            result_array = re.findall("typedef enum.*?\{.*?\}(.*?);", file_content)
            if result_array:
                valid_document = True
                for result in result_array:
                    enum_list.append(result.replace(" ", ""))
        except:
            print(f"file_name:{file_name}")
            traceback.print_exc()
            return valid_document      
    return valid_document


def _find_enum_list_from_file_list(file_list):
    enum_list = []
    include_list = []
    for file_name in file_list:
        valid_document = _find_enum_list_from_file(enum_list, file_name)
        if valid_document == True:
            include_list.append(file_name)
    return enum_list, include_list


def _find_enum_use_in_msg_struct(struct_table, enum_list):
    struct_type_list = {}
    for struct_key in struct_table:
        msg_struct = struct_table[struct_key]
        for msg_member in msg_struct:
            struct_type_list[msg_member['t']] = msg_member['t']

    new_enum_list = []
    for enum in enum_list:
        if struct_type_list.get(enum, None) != None:
            new_enum_list.append(enum)

    return new_enum_list


# =====================================================================


def find_enum_table():
    file_list = find_file_list()
    enum_list, include_list = _find_enum_list_from_file_list(file_list)

    _, msg_struct_table, _ = find_msg_struct_table()
    other_struct_table, _ = find_other_struct_table()
    struct_table = {}
    struct_table.update(msg_struct_table)
    struct_table.update(other_struct_table)
    enum_list = _find_enum_use_in_msg_struct(struct_table, enum_list)

    return enum_list, include_list
# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import json
import re
import traceback
from .find_msg_file_list import find_msg_file_list
from autocode_tools import *


def _find_struct_table_from_struct_data(msg_table, struct_table, struct_data):
    msg_name = re.findall("\/\* for *(.+?) *?message *\*\/.*?", struct_data)
    struct_data = re.findall("\/\* for *.+? *?message *\*\/.*?(typedef struct.*?\{.*?\}.*?;)", struct_data)

    msg_name = get_array_data(msg_name, 0).replace(" ", "")
    struct_name, base_array = get_struct_data(get_array_data(struct_data, 0))

    if msg_name != None:
        msg_table[msg_name] = struct_name
        struct_table[struct_name] = base_array
    return


def _find_struct_table_from_struct_list(msg_table, struct_table, struct_list):
    for struct_data in struct_list:
        _find_struct_table_from_struct_data(msg_table, struct_table, struct_data)
    return


def _find_struct_list_from_file(struct_list, include_list, file_name):
    with open(file_name, "r", encoding="utf-8") as file_id:
        try:
            file_content = file_id.read()
        except:
            print(f"file_name:{file_name}")
            traceback.print_exc()
            return struct_list, include_list
        try:
            file_content = remove_invalid_data(file_content)
            result = re.findall("\/\* for *.+? *?message *\*\/.*?typedef struct.*?\{.*?\}.*?;", file_content)
            if result:
                struct_list.extend(result)
                include_list.append(file_name)
        except:
            print(f"file_name:{file_name}")
            traceback.print_exc()
            return struct_list, include_list        
    return struct_list, include_list


def _find_struct_list_from_file_list(file_list):
    struct_list = []
    include_list = []
    for file_name in file_list:
         _find_struct_list_from_file(struct_list, include_list, file_name)
    return struct_list, include_list


# =====================================================================


def find_msg_struct_table(file_list):
    msg_file_list = find_msg_file_list(file_list)
    struct_list, include_list = _find_struct_list_from_file_list(msg_file_list)

    struct_table = {}
    msg_table = {}
    _find_struct_table_from_struct_list(msg_table, struct_table, struct_list)

    return msg_table, struct_table, include_list
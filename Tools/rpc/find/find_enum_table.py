# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import re
import traceback
from rpc_tools import *
from .find_file_list import *
from .find_all_struct_table import *


def _find_enum_body(body_content):
    body_array = re.findall("(.*?)[',','{','}']", body_content)
    body_array = [var for var in body_array if var]

    body_table = {}
    for body in body_array:
        body = body.split('=')
        if len(body) == 1:
            body_table[body[0]] = ''
        elif len(body) == 2:
            body_table[body[0]] = body[1]
        else:
            print(f'invalid body:{body}')

    return body_table


def _find_enum_name_and_body(enum_list, name_array, body_array):
    for index in range(len(name_array)):
        enum_list[name_array[index].replace(' ', '')] = _find_enum_body(body_array[index].replace(' ', ''))
    return


def _find_enum_list_from_file(enum_list, include_list, file_name):
    with open(file_name, "r", encoding="utf-8") as file_id:
        try:
            file_content = file_id.read()
        except:
            print(f"file_name:{file_name}")
            traceback.print_exc()
            return
        try:
            file_content = remove_annotation_data(file_content)
            name_array = re.findall("typedef enum.*?\{.*?\}(.*?);", file_content)
            body_array = re.findall("typedef enum.*?(\{.*?\}).*?;", file_content)
            if name_array:
                _find_enum_name_and_body(enum_list, name_array, body_array)
                include_list.append(file_name)
        except:
            print(f"file_name:{file_name}")
            traceback.print_exc()
            return
    return


def _find_enum_list_from_file_list(file_list):
    enum_list = {}
    include_list = []
    for file_name in file_list:
        _find_enum_list_from_file(enum_list, include_list, file_name)
    return enum_list, include_list


def _find_enum_use_in_msg_struct(struct_table, enum_list):
    struct_type_list = {}
    for struct_key in struct_table:
        msg_struct = struct_table[struct_key]
        for msg_member in msg_struct:
            struct_type_list[msg_member['t']] = msg_member['t']

    enum_table = {}
    for enum_name in enum_list.keys():
        if struct_type_list.get(enum_name, None) != None:
            enum_table[enum_name] = enum_list[enum_name]

    return enum_table


# =====================================================================


def find_enum_table(struct_table=None):
    file_list = find_file_list()
    enum_list, include_list = _find_enum_list_from_file_list(file_list)
    if struct_table == None:
        struct_table = find_all_struct_table()

    enum_table = _find_enum_use_in_msg_struct(struct_table, enum_list)

    return enum_table, include_list
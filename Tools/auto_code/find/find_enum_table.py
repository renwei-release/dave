# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import re
import traceback
from autocode_tools import *
from .find_file_list import *


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


def _find_enum_name_and_body(enum_table, name_array, body_array, type_array):
    is_valid_enum_data = False
    for index in range(len(name_array)):
        enum_name = name_array[index].replace(' ', '')
        enum_body = body_array[index].replace(' ', '')
        if enum_name in type_array:
            enum_table[enum_name] = _find_enum_body(enum_body)
            is_valid_enum_data = True
    return is_valid_enum_data


def _find_enum_list_from_file(enum_table, include_list, type_array, file_name):
    with open(file_name, "r", encoding="utf-8") as file_id:
        try:
            file_content = file_id.read()
        except:
            print(f"1 _find_enum_list_from_file file_name:{file_name}")
            return
        try:
            file_content = remove_annotation_data(file_content)
            name_array = re.findall("typedef enum.*?\{.*?\}(.*?);", file_content)
            body_array = re.findall("typedef enum.*?(\{.*?\}).*?;", file_content)
            if name_array:
                if _find_enum_name_and_body(enum_table, name_array, body_array, type_array) == True:
                    include_list.append(file_name)
        except:
            print(f"2 _find_enum_list_from_file file_name:{file_name}")
            return
    return


def _find_enum_list_from_file_list(type_array, file_list):
    enum_table = {}
    include_list = []
    for file_name in file_list:
        _find_enum_list_from_file(enum_table, include_list, type_array, file_name)
    return enum_table, include_list


def _find_struct_type_array(struct_table):
    type_array = []
    for struct_key in struct_table:
        msg_struct = struct_table[struct_key]
        for msg_member in msg_struct:
            type_array.append(msg_member['t'])
    return type_array


# =====================================================================


def find_enum_table(param, file_list=None, struct_table=None):
    if file_list == None:
        file_list = param['file_list']
    if struct_table == None:
        struct_table = param['all_struct_table']

    type_array = _find_struct_type_array(struct_table)
    enum_table, include_list = _find_enum_list_from_file_list(type_array, file_list)

    return enum_table, include_list
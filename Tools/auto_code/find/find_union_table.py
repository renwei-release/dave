# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import re
import traceback
from .find_file_list import *
from autocode_tools import *


def _find_union_list_from_file(union_list, file_name):
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
            result = re.findall("typedef union.*?\{.*?\}.*?;", file_content)
            if result:
                valid_document = True
                union_list.extend(result)
        except:
            print(f"file_name:{file_name}")
            traceback.print_exc()
            return valid_document      
    return valid_document


def _find_union_list_from_file_list(file_list):
    union_list = []
    include_list = []
    for file_name in file_list:
        valid_document = _find_union_list_from_file(union_list, file_name)
        if valid_document == True:
            include_list.append(file_name)
    return union_list, include_list


def _find_union_data_to_table(union_table, struct_data):
    struct_name, base_array = get_struct_data(struct_data)
    if struct_name != None:
        union_table[struct_name] = base_array
    return


def _find_union_list_to_table(struct_list):
    union_table = {}
    for struct_data in struct_list:
        _find_union_data_to_table(union_table, struct_data)
    return union_table


# =====================================================================


def find_union_table():
    file_list = find_file_list()
    union_list, include_list = _find_union_list_from_file_list(file_list)
    union_table = _find_union_list_to_table(union_list)
    return union_table, include_list
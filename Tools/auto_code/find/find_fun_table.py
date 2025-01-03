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


def _find_fun_list_from_file(fun_list, file_name, all_struct_table):
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
            result_array = re.findall("typedef void \(\*.*?(.*?)\).*?;", file_content)
            if result_array:
                for result in result_array:
                    if type_on_the_table(result, all_struct_table) == True:
                        valid_document = True
                        fun_list.append(result.replace(" ", ""))
        except:
            print(f"file_name:{file_name}")
            traceback.print_exc()
            return valid_document      
    return valid_document


def _find_fun_list_from_file_list(file_list, all_struct_table):
    fun_list = []
    include_list = []
    for file_name in file_list:
        valid_document = _find_fun_list_from_file(fun_list, file_name, all_struct_table)
        if valid_document == True:
            include_list.append(file_name)
    return fun_list, include_list


# =====================================================================


def find_fun_table(file_list, all_struct_table):
    fun_table, include_list = _find_fun_list_from_file_list(file_list, all_struct_table)
    return fun_table, include_list
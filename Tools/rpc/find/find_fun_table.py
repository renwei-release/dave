# -*- coding: utf-8 -*-

#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.03.11.
#

import re
import traceback
from .find_file_list import *
from rpc_tools import *


def _find_fun_list_from_file(fun_list, file_name):
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
                valid_document = True
                for result in result_array:
                    fun_list.append(result.replace(" ", ""))
        except:
            print(f"file_name:{file_name}")
            traceback.print_exc()
            return valid_document      
    return valid_document


def _find_fun_list_from_file_list(file_list):
    fun_list = []
    include_list = []
    for file_name in file_list:
        valid_document = _find_fun_list_from_file(fun_list, file_name)
        if valid_document == True:
            include_list.append(file_name)
    return fun_list, include_list


# =====================================================================


def find_fun_table():
    file_list = find_file_list()
    fun_table, include_list = _find_fun_list_from_file_list(file_list)
    return fun_table, include_list
# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from .find_other_struct_table import find_other_struct_table
from .find_msg_struct_table import find_msg_struct_table


# =====================================================================


def find_all_struct_table(file_list):
    msg_name_table, msg_struct_table, msg_include_list = find_msg_struct_table(file_list)
    other_struct_table, other_include_list = find_other_struct_table(file_list, msg_struct_table)

    all_struct_table = msg_struct_table.copy()
    all_struct_table.update(other_struct_table)

    param = {}
    param['all_struct_table'] = all_struct_table
    param['msg_name_table'] = msg_name_table
    param['msg_struct_table'] = msg_struct_table
    param['msg_include_list'] = msg_include_list
    param['other_struct_table'] = other_struct_table
    param['other_include_list'] = other_include_list

    return param
# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from find.find_file_list import find_file_list
from find.find_all_struct_table import find_all_struct_table
from find.find_define_table import find_define_table
from find.find_meta_table import find_meta_table
from find.find_enum_table import find_enum_table


# =====================================================================


def find_param():
    param = {}

    file_list = find_file_list()
    if file_list == None:
        return None

    param['file_list'] = file_list

    param.update(find_all_struct_table(file_list))

    define_table, _ = find_define_table(file_list, param['all_struct_table'])
    param['define_table'] = define_table

    param['meta_table'] = find_meta_table()

    enum_table, enum_list, total_enum_table, total_include_list = find_enum_table(param)
    param['enum_table'] = enum_table
    param['enum_list'] = enum_list
    param['total_enum_table'] = total_enum_table
    param['total_enum_list'] = total_include_list

    return param
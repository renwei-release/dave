# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from find.find_param import find_param
from find.find_enum_table import find_enum_table
from enumstr.c.c_creat_enumstr_file import *
from enumstr.go.go_creat_enumstr_file import *
from enumstr.python.py_creat_enumstr_file import *


def _enumstr_load_rpc_h_data():
    file_list = [f'{rpc_ver3_rpcinc_file_name}']
    struct_table = {"Simulate_use_RPCMSG_struct" : struct_table_set(None, [{ 'n':"Simulate_use_RPCMSG_name", 't':"RPCMSG" }])}

    enum_table, _, _, _ = find_enum_table(None, file_list, struct_table)
    return enum_table


def _enumstr_load_enum_table(param):
    total_enum_table = param['total_enum_table']
    total_include_list = param['total_enum_list']
    enum_table = param['enum_table']
    include_list = param['enum_list']

    total_enum_table.update(_enumstr_load_rpc_h_data())
    enum_table.update(_enumstr_load_rpc_h_data())

    return total_enum_table, total_include_list, enum_table, include_list


# =====================================================================


def enumstr_main(param):
    total_enum_table, total_include_list, enum_table, include_list = _enumstr_load_enum_table(param)

    creat_c_enumstr_file(total_enum_table, total_include_list)
    creat_go_enumstr_file(enum_table, include_list)
    creat_py_enumstr_file(enum_table, include_list)
    return


if __name__ == '__main__':
    param = find_param()

    enumstr_main(param)
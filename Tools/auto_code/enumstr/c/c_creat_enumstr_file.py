# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from autocode_cfg import *
from autocode_tools import *
from find.find_enum_table import find_enum_table


def _creat_c_enumstr_body_table(body_table, file_id):
    for body_name in body_table:
        #
        # The first byte is '_' is a special definition, 
        # no need to automatically generate code, 
        # such as: __ErrCode_last_xxxx__
        #
        body_value = body_table[body_name]
        if body_name[0] != '_':
            if (body_value == '') \
                or (is_digital_string(body_value) == True):
                file_id.write(f'\t\tcase {body_name}:\n')
                if body_value == '':
                    file_id.write(f'\t\t\t\tvalue_str = "\'{body_name}\'";\n')
                else:
                    file_id.write(f'\t\t\t\tvalue_str = "\'{body_name}:{body_value}\'";\n')
                file_id.write(f'\t\t\tbreak;\n')
    file_id.write(f'\t\tdefault:\n')
    file_id.write(f'\t\t\t\tvalue_str = "\'NULL\'";\n')
    file_id.write(f'\t\t\tbreak;\n')
    return


def _load_audocode_rpc_h_data():
    file_list = [f'{rpc_ver3_rpcinc_file_name}']
    struct_table = {"Simulate_use_RPCMSG_struct" : struct_table_set(None, [{ 'n':"Simulate_use_RPCMSG_name", 't':"RPCMSG" }])}

    enum_table, _, _, _ = find_enum_table(None, file_list, struct_table)
    return enum_table


def _creat_c_enumstr_src_file(enum_table, include_list, file_name):
    with open(file_name, "w+", encoding="utf-8") as file_id:
        copyright_message(file_id)
        file_id.write(f'#include "dave_base.h"\n')
        file_id.write(f'#include "dave_tools.h"\n')
        include_message(file_id, include_list)
        dividing_line(file_id)
        for enum_name in enum_table:
            file_id.write(f'\ns8 *\n')
            file_id.write(f't_auto_{enum_name}_str({enum_name} enum_value)\n')
            file_id.write(f'{"{"}\n')
            file_id.write(f'\ts8 *value_str = NULL;\n\n')
            file_id.write(f'\tswitch(enum_value)\n')
            file_id.write(f'\t{"{"}\n')
            _creat_c_enumstr_body_table(enum_table[enum_name], file_id)
            file_id.write(f'\t{"}"}\n')
            file_id.write(f'\n\treturn value_str;\n')
            file_id.write(f'{"}"}\n')
    return


def _creat_c_enumstr_inc_file(enum_table, include_list, file_name):
    with open(file_name, "w+", encoding="utf-8") as file_id:
        copyright_message(file_id)
        file_id.write(f'#ifndef __T_AUTO_ENUMSTR__\n')
        file_id.write(f'#define __T_AUTO_ENUMSTR__\n')
        file_id.write(f'#include "dave_base.h"\n')
        include_message(file_id, include_list)

        for enum_name in enum_table:
            file_id.write(f's8 *t_auto_{enum_name}_str({enum_name} enum_value);\n')

        file_id.write(f'\n#endif\n')
    return


# =====================================================================


def creat_c_enumstr_file(param):
    enum_table = param['total_enum_table']
    include_list = param['total_enum_list']

    enum_table.update(_load_audocode_rpc_h_data())

    print(f"{len(enum_table)}\tenumdata\twrite to {c_enumstr_src_file_name}")
    _creat_c_enumstr_src_file(enum_table, include_list, c_enumstr_src_file_name)
    _creat_c_enumstr_inc_file(enum_table, include_list, c_enumstr_inc_file_name)
    return
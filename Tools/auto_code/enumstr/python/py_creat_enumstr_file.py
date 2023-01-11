# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from autocode_cfg import *
from autocode_tools import *


def _enumstr_body_table(body_table, file_id):
    frist_if_logic = True

    for body_name in body_table:
        #
        # The first byte is '_' is a special definition, 
        # no need to automatically generate code, 
        # such as: __ErrCode_last_xxxx__
        #
        body_value = body_table[body_name]
        if body_name[0] != '_':
            if (body_value == '') or (is_digital_string(body_value) == True):
                if frist_if_logic == True:
                    frist_if_logic = False
                    file_id.write(f'\tif enum_value == {body_name}:\n')
                else:
                    file_id.write(f'\telif enum_value == {body_name}:\n')
                if body_value == '':
                    file_id.write(f'\t\tvalue_str = "\'{body_name}\'"\n')
                else:
                    file_id.write(f'\t\tvalue_str = "\'{body_name}-{body_value}\'"\n')
    file_id.write("\telse:\n\t\tvalue_str = f'{enum_value}'\n")
    return


def _enumstr_src_file(enum_table, include_list, file_name):
    with open(file_name, "w+", encoding="utf-8") as file_id:
        copyright_message(file_id, 'python')
        file_id.write(f'from .dave_enum import *\n\n')
        for enum_name in enum_table:
            file_id.write(f'def t_auto_{enum_name}_str(enum_value):\n')
            _enumstr_body_table(enum_table[enum_name], file_id)
            file_id.write(f'\treturn value_str\n\n\n')
    return


# =====================================================================


def creat_py_enumstr_file(enum_table, include_list):
    print(f"{len(enum_table)}\tenumdata\twrite to {py_enumstr_src_file_name}")
    _enumstr_src_file(enum_table, include_list, py_enumstr_src_file_name)
    return
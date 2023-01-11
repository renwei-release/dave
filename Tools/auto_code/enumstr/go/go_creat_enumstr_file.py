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
    for body_name in body_table:
        #
        # The first byte is '_' is a special definition, 
        # no need to automatically generate code, 
        # such as: __ErrCode_last_xxxx__
        #
        body_value = body_table[body_name]
        if body_name[0] != '_':
            if (body_value == '') or (is_digital_string(body_value) == True):
                file_id.write(f'\t\tcase {body_name}: ')
                if body_value == '':
                    file_id.write(f'value_str = "\'{body_name}\'"\n')
                else:
                    file_id.write(f'value_str = "\'{body_name}-{body_value}\'"\n')
    file_id.write(f'\t\tdefault: value_str = fmt.Sprintf("\'%v\'", enum_value)\n')
    return


def _enumstr_src_file(enum_table, include_list, file_name):
    with open(file_name, "w+", encoding="utf-8") as file_id:
        file_id.write(f'package auto\n')
        copyright_message(file_id)
        file_id.write(f'import (\n')
        file_id.write(f'   "fmt"\n')
        file_id.write(f')\n\n')
        for enum_name in enum_table:
            file_id.write(f'func T_auto_{enum_name}_str(enum_value int64) string {"{"}\n')
            file_id.write(f'\tvar value_str string\n\n')
            file_id.write(f'\tswitch enum_value {"{"}\n')
            _enumstr_body_table(enum_table[enum_name], file_id)
            file_id.write("\t}\n")
            file_id.write(f'\n\treturn value_str\n')
            file_id.write(f'{"}"}\n\n')
    return


# =====================================================================


def creat_go_enumstr_file(enum_table, include_list):
    print(f"{len(enum_table)}\tenumdata\twrite to {go_enumstr_src_file_name}")
    _enumstr_src_file(enum_table, include_list, go_enumstr_src_file_name)
    return
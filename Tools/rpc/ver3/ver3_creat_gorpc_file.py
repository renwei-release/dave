# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from rpc_cfg import *
from rpc_tools import *
from find.find_other_struct_table import find_other_struct_table
from find.find_msg_struct_table import find_msg_struct_table
from find.find_define_table import find_define_table
from find.find_all_struct_table import find_all_struct_table


def _c_enum_to_go_type(enum_value_array):
    for key in enum_value_array.keys():
        if enum_value_array[key] != '':
            if '0x' in enum_value_array[key]:
                if int(enum_value_array[key], 16) > 4294967295:
                    return 'int64'
            elif '0X' in enum_value_array[key]:
                if int(enum_value_array[key], 16) > 4294967295:
                    return 'int64'
            else:
                if int(enum_value_array[key]) > 4294967295:
                    return 'int64'            
    return 'int32'


def _c_type_to_go_type(c_type, struct_total, enum_table):
    if c_type == 'dave_bool':
        return 'int8'
    elif c_type == 's8':
        return 'byte'
    elif c_type == 'u8':
        return 'byte'
    elif c_type == 's16':
        return 'int16'
    elif c_type == 'u16':
        return 'uint16'
    elif c_type == 's32':
        return 'int32'
    elif c_type == 'u32':
        return 'uint32'
    elif c_type == 's64':
        return 'int64'
    elif c_type == 'u64':
        return 'uint64'
    elif c_type == 'sb':
        return 'int64'
    elif c_type == 'ub':
        return 'uint64'
    elif c_type == 'ThreadId':
        return 'uint64'
    elif c_type == 'void_ptr':
        return 'unsafe.Pointer'
    elif c_type == 'MBUF_ptr':
        return '*MBUF'
    elif enum_table.get(c_type, None) != None:
        return _c_enum_to_go_type(enum_table[c_type])
    elif struct_total.get(c_type, None) != None:
        return c_type

    print(f'Types:{c_type} that cannot be handled!')
    return c_type


def _creat_define_file(define_table, file_name):
    print(f"{len(define_table)}\tdefine\t\twrite to {file_name}")
    with open(file_name, "w+", encoding="utf-8") as file_id:
        file_id.write('package base\n')
        copyright_message(file_id)

        for key in define_table.keys():
            file_id.write(f'const {key} = {define_table[key]}\n')
    return


def _creat_msg_id_file(msg_id_table, file_name):
    print(f'{len(msg_id_table)}\tmsgid\t\twrite to {file_name}')
    with open(file_name, "w+", encoding="utf-8") as file_id:
        file_id.write('package base\n')
        copyright_message(file_id)

        file_id.write('const (\n')
        for key in msg_id_table.keys():
            file_id.write(f'\t{key} = {msg_id_table[key]}\n')
        file_id.write(')\n\n')
    return


def _creat_struct_file(struct_table, struct_total, enum_table, file_name):
    print(f'{len(struct_table)}\tstruct\t\twrite to {file_name}')
    with open(file_name, "w+", encoding="utf-8") as file_id:
        file_id.write('package base\n')
        copyright_message(file_id)
        file_id.write('\nimport "unsafe"\n\n')

        for struct_name in struct_table.keys():
            file_id.write(f'type {struct_name} struct {"{"}\n')
            for struct_data in struct_table[struct_name]:
                value_name = struct_data['n'].title()
                value_type = struct_data['t']
                value_dimension = struct_data.get('d', None)
                if value_dimension == None:
                    file_id.write(f'\t{value_name} {_c_type_to_go_type(value_type, struct_total, enum_table)}\n')
                else:
                    file_id.write(f'\t{value_name} [{value_dimension}] {_c_type_to_go_type(value_type, struct_total, enum_table)}\n')
            file_id.write(f'{"}"}\n\n')
    return


def _creat_enum_file(enum_table, file_name):
    print(f'{len(enum_table)}\tenum\t\twrite to {file_name}')
    with open(file_name, "w+", encoding="utf-8") as file_id:
        file_id.write('package base\n')
        copyright_message(file_id)

        for enum_name in enum_table.keys():
            frist_note = True
            file_id.write(f'const (\n')
            for enum_key in enum_table[enum_name].keys():
                if frist_note == True:
                    frist_note = False
                    if enum_table[enum_name][enum_key] == '':
                        file_id.write(f'\t{enum_key} int = iota\n')
                    else:
                        file_id.write(f'\t{enum_key} = {enum_table[enum_name][enum_key]} + iota\n')
                else:
                    if enum_table[enum_name][enum_key] == '':
                        file_id.write(f'\t{enum_key}\n')
                    else:
                        file_id.write(f'\t{enum_key} = {enum_table[enum_name][enum_key]}\n')                   
            file_id.write(f')\n\n')
    return


# =====================================================================


def creat_gorpc_file(struct_table, msg_struct_table, msg_id_table, enum_table):
    define_table, _ = find_define_table()
    struct_total = find_all_struct_table()

    _creat_define_file(define_table, ver3_godefine_file_name)
    _creat_msg_id_file(msg_id_table, ver3_gomsgid_file_name)
    _creat_struct_file(struct_table, struct_total, enum_table, ver3_gostruct_file_name)
    _creat_struct_file(msg_struct_table, struct_total, enum_table, ver3_gomsgstruct_file_name)
    _creat_enum_file(enum_table, ver3_goenum_file_name)
    return
# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei zhaojie chenxiaomin
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
# *
# * 20220430 --- zhaojie chenxiaomin
# * Fix the error in the type judgment of the enumeration processing
# * function:_c_enum_to_go_type
# *
from autocode_cfg import *
from autocode_tools import *

go_package_name = 'package auto\n'

def _c_enum_to_go_type(enum_value_array):
    for key in enum_value_array.keys():
        value = enum_value_array[key]
        if (value != '') and (is_digital_string(value) == True):
            if '0x' in value:
                if int(value, 16) > 4294967295:
                    return 'int64'
            elif '0X' in value:
                if int(value, 16) > 4294967295:
                    return 'int64'
            else:
                if int(value) > 4294967295:
                    return 'int64'            
    return 'int32'


def _c_type_to_go_type(c_type, is_ptr, struct_total, enum_table, fun_table):
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
    elif c_type == 'char':
        return 'byte'
    elif c_type == 'singed char':
        return 'int8'
    elif c_type == 'unsigned char':
        return 'uint8'
    elif c_type == 'short':
        return 'int16'
    elif c_type == 'unsigned short':
        return 'uint16'
    elif c_type == 'int':
        return 'int32'
    elif c_type == 'unsigned int':
        return 'uint32'
    elif c_type == 'long':
        return 'int32'
    elif c_type == 'unsigned long':
        return 'uint32'
    elif c_type == 'long long int':
        return 'int64'
    elif c_type == 'unsigned long long int':
        return 'uint64'
    elif c_type == 'float':
        return 'float32'
    elif c_type == 'double':
        return 'float64'
    elif c_type == 'void':
        if is_ptr == True:
            return 'unsafe.Pointer'
        else:
            return 'uint64'
    elif enum_table.get(c_type, None) != None:
        return _c_enum_to_go_type(enum_table[c_type])
    elif struct_total.get(c_type, None) != None:
        if is_ptr == True:
            return '*' + c_type
        else:
            return c_type
    elif c_type in fun_table:
        return c_type

    print(f'_c_type_to_go_type c_type:{c_type} that cannot be handled!')
    return c_type


def _creat_define_file(define_table, file_name):
    print(f"{len(define_table)}\tdefine\t\twrite to {file_name}")
    with open(file_name, "w+", encoding="utf-8") as file_id:
        file_id.write(go_package_name)
        copyright_message(file_id)

        for key in define_table.keys():
            file_id.write(f'const {key} = {define_table[key]}\n')
    return


def _creat_msg_id_file(msg_id_table, file_name):
    print(f'{len(msg_id_table)}\tmsgid\t\twrite to {file_name}')
    with open(file_name, "w+", encoding="utf-8") as file_id:
        file_id.write(go_package_name)
        copyright_message(file_id)

        file_id.write('const (\n')
        for key in msg_id_table.keys():
            file_id.write(f'\t{key} = {msg_id_table[key]}\n')
        file_id.write(')\n\n')
    return


def _creat_msg_id_information(file_id, msg_name):
    file_id.write(f'/* for {msg_name} message */\n')
    return


def _creat_struct_file(struct_table, struct_total, enum_table, fun_table, file_name):
    print(f'{len(struct_table)}\tstruct\t\twrite to {file_name}')
    with open(file_name, "w+", encoding="utf-8") as file_id:
        file_id.write(go_package_name)
        copyright_message(file_id)
        file_id.write('import "unsafe"\n\n')

        for struct_name in struct_table.keys():
            _creat_msg_id_information(file_id, struct_table_get(struct_table[struct_name], 'msg_name'))
            file_id.write(f'type {struct_name} struct {"{"}\n')
            for struct_data in struct_table_get(struct_table[struct_name]):
                value_name = struct_data['n'].capitalize()
                value_type = struct_data['t']
                value_dimension = struct_data['d']
                is_ptr = struct_data['p']
                if value_dimension == None:
                    file_id.write(f'\t{value_name} {_c_type_to_go_type(value_type, is_ptr, struct_total, enum_table, fun_table)}\n')
                else:
                    file_id.write(f'\t{value_name} [{value_dimension}] {_c_type_to_go_type(value_type, is_ptr, struct_total, enum_table, fun_table)}\n')
            file_id.write(f'{"}"}\n\n')
    return


def _creat_enum_file(enum_table, file_name):
    print(f'{len(enum_table)}\tenum\t\twrite to {file_name}')
    with open(file_name, "w+", encoding="utf-8") as file_id:
        file_id.write(go_package_name)
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


def creat_gorpc_file(param):
    all_struct_table = param['all_struct_table']
    other_struct_table = param['other_struct_table']
    msg_struct_table = param['msg_struct_table']
    define_table = param['define_table']
    msg_id_table = param['msg_id_table']
    enum_table = param['enum_table']
    fun_table = param['fun_table']

    _creat_define_file(define_table, rpc_ver3_godefine_file_name)
    _creat_msg_id_file(msg_id_table, rpc_ver3_gomsgid_file_name)
    _creat_struct_file(other_struct_table, all_struct_table, enum_table, fun_table, rpc_ver3_gostruct_file_name)
    _creat_struct_file(msg_struct_table, all_struct_table, enum_table, fun_table, rpc_ver3_gomsgstruct_file_name)
    _creat_enum_file(enum_table, rpc_ver3_goenum_file_name)
    return
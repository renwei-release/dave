# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from autocode_cfg import *
from find.find_all_struct_table import find_all_struct_table
from find.find_enum_table import find_enum_table
from find.find_fun_table import find_fun_table


def _creat_type_proto(type, struct_table, enum_table, fun_table):
    if type == 'dave_bool':
        proto_type = 'bool'
    elif type == 'char':
        proto_type = 'int32'
    elif type == 'unsigned char':
        proto_type = 'uint32'
    elif type == 'float':
        proto_type = 'float'
    elif type == 'float_ptr':
        proto_type = 'uint64'
    elif type == 'double':
        proto_type = 'double'
    elif type == 's8':
        proto_type = 'int32'
    elif type == 's8_ptr':
        proto_type = 'string'
    elif type == 'u8':
        proto_type = 'uint32'
    elif type == 'u8_ptr':
        proto_type = 'uint64'
    elif type == 'int':
        proto_type = 'int32'
    elif type == 's16':
        proto_type = 'int32'  
    elif type == 'u16':
        proto_type = 'uint32'
    elif type == 's32':
        proto_type = 'int32'
    elif type == 'u32':
        proto_type = 'uint32'
    elif type == 'sb':
        proto_type = 'int64'
    elif type == 'ub':
        proto_type = 'uint64'
    elif type == 'ThreadId':
        proto_type = 'uint64'
    elif enum_table.get(type, None) != None:
        proto_type = 'uint64'
    elif type in struct_table:
        proto_type = type
    elif type in fun_table:
        proto_type = 'uint64'
    elif type == 'void_ptr':
        proto_type = 'uint64'
    elif type == 'MBUF_ptr':
        proto_type = 'dave_mbuf'
    elif type == 'IIBase_ptr':
        proto_type = 'uint64'
    elif type == 'NLPSegmentor_ptr':
        proto_type = 'uint64'
    elif type == 'NLPNER_ptr':
        proto_type = 'uint64'
    else:
        proto_type = None

    return proto_type


def _creat_struct_type(one_struct, struct_table, enum_table, fun_table, file_id):
    type = one_struct['t']

    proto_type = _creat_type_proto(type, struct_table, enum_table, fun_table)
    if proto_type != None:
        file_id.write(f'\t {proto_type} ')
        return True
    else:
        return False


def _creat_struct_name(one_struct, file_id):
    name = one_struct['n']
    file_id.write(f'{name} ')
    return


def _creat_struct_index(struct_index, file_id):
    file_id.write(f'= {struct_index};\n')
    return


def _creat_struct_data(struct_data, struct_table, enum_table, fun_table, file_id):
    struct_index = 1
    for one_struct in struct_data:
        if _creat_struct_type(one_struct, struct_table, enum_table, fun_table, file_id) == True:
            _creat_struct_name(one_struct, file_id)
            _creat_struct_index(struct_index, file_id)
            struct_index += 1

    return


def _creat_message_data(struct_name, struct_data, struct_table, enum_table, fun_table, file_id):
    file_id.write(f'message {struct_name} {{\n')
    _creat_struct_data(struct_data, struct_table, enum_table, fun_table, file_id)
    file_id.write(f'}}\n\n')
    return


def _creat_proto_file(struct_table, enum_table, fun_table, file_name):
    with open(file_name, "w+", encoding="utf-8") as file_id:
        file_id.write("syntax = \"proto3\";\n\n")
        for struct_name in struct_table.keys():
            struct_data = struct_table[struct_name]
            _creat_message_data(struct_name, struct_data, struct_table, enum_table, fun_table, file_id)
    return


# =====================================================================


def creat_proto_file(struct_table=None):
    if struct_table == None:
        struct_table = find_all_struct_table()
    enum_table, _ = find_enum_table()
    fun_table, _ = find_fun_table()

    print(f"{len(struct_table)}\tproto\t\twrite to {rpc_ver2_proto_file_name}")
    _creat_proto_file(struct_table, enum_table, fun_table, rpc_ver2_proto_file_name)
    return rpc_ver2_proto_file_name
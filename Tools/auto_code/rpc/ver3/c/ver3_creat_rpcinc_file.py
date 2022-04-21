# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from autocode_cfg import *
from autocode_tools import *
from find.find_msg_id_list import find_msg_id_list


_rpcinc_head = "\
#ifndef __T_RPC_H__\n\
#define __T_RPC_H__\n\n"

_rpcinc_end = "\
MBUF * t_rpc_zip(sb ver, ub msg_id, void *msg_body, ub msg_len);\n\
dave_bool t_rpc_unzip(void **msg_body, ub *msg_len, ub msg_id, s8 *packet_ptr, ub packet_len);\n\
\n\
#endif\n\n"


def _read_rpcinc_file(file_name):
    msg_id_exist_table = {}
    with open(file_name, encoding="utf-8") as file_id:
        file_content = remove_annotation_data(file_id.read())
        result = re.findall("(typedef enum.*?\{.*?\}.*?;)", file_content)
        key_array = re.findall('([A-Z,a-z,0-9,_]*) = [0-9]*,', str(result))
        value_array = re.findall('[A-Z,a-z,0-9,_]* = ([0-9]*),', str(result))

    index = 0
    for key in key_array:
        has_value = re.findall('[A-Z,a-z,0-9,_]* = ([0-9])*,', value_array[index])
        if has_value != None:
            msg_id_exist_table[key] = value_array[index]
        index += 1
    return msg_id_exist_table


def _write_exist_enum(msg_id_exist_table, file_id):
    if len(msg_id_exist_table) == 0:
        return 1

    key = ''
    for key in msg_id_exist_table:
        file_id.write(f'\t{key} = {msg_id_exist_table[key]},\n')

    if key == None:
        return 1
    msg_value = msg_id_exist_table[key]
    return int(msg_value) + 1


def _creat_rpcinc_file(msg_id_list, file_name):
    msg_id_exist_table = _read_rpcinc_file(file_name)
    msg_id_table = msg_id_exist_table.copy()

    with open(file_name, "w+", encoding="utf-8") as file_id:
        copyright_message(file_id)
        file_id.write(_rpcinc_head)
        file_id.write('typedef enum {\n')
        file_id.write('\tMSGID_RESERVED = 0x0000000000000000,\n\n')
        msg_id_table['MSGID_RESERVED'] = 0x0000000000000000

        msg_value = _write_exist_enum(msg_id_exist_table, file_id)
        for msg_id in msg_id_list:
            msg_id_exist = msg_id_exist_table.get(msg_id, None)
            if msg_id_exist == None:
                file_id.write(f'\t{msg_id} = {msg_value},\n')
                msg_id_table[msg_id] = msg_value
                msg_value += 1

        file_id.write('\n\tMSGID_INVALID = 0xffffffffffffffff\n')
        msg_id_table['MSGID_INVALID'] = 0xffffffffffffffff
        file_id.write('} RPCMSG;\n\n')
        file_id.write(_rpcinc_end)
    return msg_id_table


def __check_if_the_table_has_the_same_value(msg_id_table, check_msg_id, check_msg_value):
    for msg_id in msg_id_table.keys():
        if msg_id == check_msg_id:
            continue
        msg_value = msg_id_table[msg_id]
        if msg_value == check_msg_value:
            print(f'\033[93mNote that the value:{msg_value} of these two messages:{check_msg_id}/{msg_id} is the same!!!\033[0m')
    return


def _check_if_the_table_has_the_same_value(msg_id_table):
    for msg_id in msg_id_table.keys():
        msg_value = msg_id_table[msg_id]
        __check_if_the_table_has_the_same_value(msg_id_table, msg_id, msg_value)
    return


# =====================================================================


def creat_rpcinc_file(param):
    file_list = param['file_list']

    msg_id_list = find_msg_id_list(file_list)
    print(f"{len(msg_id_list)}\tmsgid\t\twrite to {rpc_ver3_rpcinc_file_name}")

    msg_id_table = _creat_rpcinc_file(msg_id_list, rpc_ver3_rpcinc_file_name)
    _check_if_the_table_has_the_same_value(msg_id_table)
    return msg_id_table
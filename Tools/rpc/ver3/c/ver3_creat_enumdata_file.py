# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from rpc_cfg import *
from rpc_tools import *
from find.find_enum_table import find_enum_table


_enumdata_src_head = "\
#include \"dave_base.h\"\n\
#include \"dave_os.h\"\n\
#include \"dave_tools.h\"\n\
#include \"dave_third_party.h\"\n\
#include \"t_rpc_ver3_metadata.h\"\n\
#include \"tools_log.h\"\n"


_enumdata_src_private = "\
\n#define DEFAULT_ENUM_VALUE -1\n\
\n\
static void *\n\
_t_rpc_zip_enumdata(sb zip_data)\n\
{\n\
    return t_rpc_ver3_zip_sb(zip_data);\n\
}\n\
\n\
static dave_bool\n\
_t_rpc_unzip_enumdata(sb *unzip_data, void *pArrayBson)\n\
{\n\
    return t_rpc_ver3_unzip_sb(unzip_data, pArrayBson);\n\
}\n"


_enumdata_inc_head = "\
#ifndef _T_RPC_ENUMDATA_H__\n\
#define _T_RPC_ENUMDATA_H__\n\
#include \"dave_base.h\"\n"\


_enumdata_inc_end = "\
#endif\n\n"


def _creat_enumdata_include_file(file_id, include_list):
    file_id.write(_enumdata_src_private)
    file_id.write("\n// =====================================================================\n\n")
    return


def _creat_enumdata_zip_fun_file(file_id, enum_name):
    file_id.write("void *\n")
    file_id.write("t_rpc_ver3_zip_"+enum_name+"("+enum_name+" zip_data)\n")
    file_id.write("{\n\treturn _t_rpc_zip_enumdata((sb)zip_data);\n}\n\n")
    return


def _creat_enumdata_unzip_fun_file(file_id, enum_name):
    file_id.write("dave_bool\nt_rpc_ver3_unzip_"+enum_name+"("+enum_name+" *unzip_data, void *pArrayBson)\n")
    file_id.write("{\n\tsb sb_unzip_data;\n\tdave_bool ret;\n\n\tret = _t_rpc_unzip_enumdata(&sb_unzip_data, pArrayBson);\n\n\t*unzip_data = ("+enum_name+")sb_unzip_data;\n\n\treturn ret;\n}\n\n")
    return


def _creat_enumdata_fun_file(file_id, enum_table):
    for enum_name in enum_table.keys():
        _creat_enumdata_zip_fun_file(file_id, enum_name)
        _creat_enumdata_unzip_fun_file(file_id, enum_name)
    return


def _creat_enumdata_src_file(enum_table, include_list, file_name):
    with open(file_name, "w+", encoding="utf-8") as file_id:
        copyright_message(file_id)
        file_id.write(_enumdata_src_head)
        include_message(file_id, include_list)
        _creat_enumdata_include_file(file_id, include_list)
        _creat_enumdata_fun_file(file_id, enum_table)
    return


def _creat_enumdata_inc_file(enum_table, include_list, file_name):
    with open(file_name, "w+", encoding="utf-8") as file_id:
        copyright_message(file_id)
        file_id.write(_enumdata_inc_head)
        include_message(file_id, include_list)
        file_id.write("\n")
        for enum_name in enum_table.keys():
            file_id.write("void * t_rpc_ver3_zip_"+enum_name+"("+enum_name+" zip_data);\n")
            file_id.write("dave_bool t_rpc_ver3_unzip_"+enum_name+"("+enum_name+" *unzip_data, void *pArrayBson);\n\n")
        file_id.write(_enumdata_inc_end)
    return


# =====================================================================


def creat_enumdata_file(struct_table=None):
    enum_table, include_list = find_enum_table(struct_table)
    print(f"{len(enum_table)}\tenumdata\twrite to {ver3_enumdata_src_file_name}")
    _creat_enumdata_src_file(enum_table, include_list, ver3_enumdata_src_file_name)
    _creat_enumdata_inc_file(enum_table, include_list, ver3_enumdata_inc_file_name)
    return enum_table
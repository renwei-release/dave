# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from autocode_cfg import *
from autocode_tools import *
from find.find_fun_table import find_fun_table


_fundata_src_head = "\
#include \"dave_base.h\"\n\
#include \"dave_os.h\"\n\
#include \"dave_tools.h\"\n\
#include \"dave_3rdparty.h\"\n\
#include \"t_rpc_ver3_metadata.h\"\n\
#include \"tools_log.h\"\n"


_fundata_src_private = "\
static void *\n\
_t_rpc_zip_fundata(void *zip_data)\n\
{\n\
    return t_rpc_ver3_zip_void_ptr(zip_data);\n\
}\n\
\n\
static dave_bool\n\
_t_rpc_unzip_fundata(void **unzip_data, void *pArrayBson)\n\
{\n\
    return t_rpc_ver3_unzip_void_ptr(unzip_data, pArrayBson);\n\
}\n"


_fundata_inc_head = "\
#ifndef _T_RPC_FUNDATA_H__\n\
#define _T_RPC_FUNDATA_H__\n\
#include \"dave_base.h\"\n"\


_fundata_inc_end = "\
#endif\n\n"


def _creat_fundata_include_file(file_id, include_list):
    file_id.write(_fundata_src_head)
    include_message(file_id, include_list)
    file_id.write(_fundata_src_private)
    file_id.write("\n// =====================================================================\n\n")
    return


def _creat_fundata_zip_fun_file(file_id, fun_name):
    file_id.write("void *\n")
    file_id.write("t_rpc_ver3_zip_"+fun_name+"("+fun_name+" zip_data)\n")
    file_id.write("{\n\treturn _t_rpc_zip_fundata((void *)zip_data);\n}\n\n")
    return


def _creat_fundata_unzip_fun_file(file_id, fun_name):
    file_id.write("dave_bool\nt_rpc_ver3_unzip_"+fun_name+"("+fun_name+" *unzip_data, void *pArrayBson)\n")
    file_id.write("{\n\treturn _t_rpc_unzip_fundata((void **)unzip_data, pArrayBson);\n}\n\n")
    return


def _creat_fundata_fun_file(file_id, fun_table):
    for fun_name in fun_table:
        _creat_fundata_zip_fun_file(file_id, fun_name)
        _creat_fundata_unzip_fun_file(file_id, fun_name)
    return


def _creat_fundata_src_file(fun_table, include_list, file_name):
    with open(file_name, "w+", encoding="utf-8") as file_id:
        copyright_message(file_id)
        if len(fun_table) > 0:
            _creat_fundata_include_file(file_id, include_list)
        _creat_fundata_fun_file(file_id, fun_table)
    return


def _creat_fundata_inc_file(fun_table, include_list, file_name):
    with open(file_name, "w+", encoding="utf-8") as file_id:
        copyright_message(file_id)
        file_id.write(_fundata_inc_head)
        include_message(file_id, include_list)
        for fun_name in fun_table:
            file_id.write("void * t_rpc_ver3_zip_"+fun_name+"("+fun_name+" zip_data);\n")
            file_id.write("dave_bool t_rpc_ver3_unzip_"+fun_name+"("+fun_name+" *unzip_data, void *pArrayBson);\n\n")
        file_id.write(_fundata_inc_end)
    return


# =====================================================================


def creat_fundata_file(param):
    file_list = param['file_list']
    all_struct_table = param['all_struct_table']

    fun_table, include_list = find_fun_table(file_list, all_struct_table)
    print(f"{len(fun_table)}\tfundata\t\twrite to {rpc_ver3_fundata_src_file_name}")
    _creat_fundata_src_file(fun_table, include_list, rpc_ver3_fundata_src_file_name)
    _creat_fundata_inc_file(fun_table, include_list, rpc_ver3_fundata_inc_file_name)
    return fun_table
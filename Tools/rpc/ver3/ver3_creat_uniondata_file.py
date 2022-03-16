# -*- coding: utf-8 -*-

#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.03.11.
#

from rpc_cfg import *
from rpc_tools import *
from find.find_union_table import find_union_table
from find.find_meta_table import find_meta_table


_uniondata_src_head = "\
#include \"dave_base.h\"\n\
#include \"dave_tools.h\"\n\
#include \"dave_third_party.h\"\n\
#include \"tools_log.h\"\n"


_uniondata_src_private = "\
#define _t_rpc_zip_uniondata(zip_data, zip_len) __t_rpc_zip_uniondata__(zip_data, zip_len, (s8 *)__func__, (ub)__LINE__)\n\
static inline void *\n\
__t_rpc_zip_uniondata__(void *zip_data, ub zip_len, s8 *fun, ub line)\n\
{\n\
	void *pArrayBson = t_bson_malloc_array();\n\
    t_bson_array_add_bin(pArrayBson, (char *)zip_data, (int)zip_len);\n\
	return pArrayBson;\n\
}\n\
\n\
#define _t_rpc_unzip_uniondata(unzip_data, unzip_len, pArrayBson) __t_rpc_unzip_uniondata__(unzip_data, unzip_len, pArrayBson, (s8 *)__func__, (ub)__LINE__)\n\
static inline dave_bool\n\
__t_rpc_unzip_uniondata__(void *unzip_data, ub unzip_len, void *pArrayBson, s8 *fun, ub line)\n\
{\n\
    size_t cpy_len;\n\
	if(pArrayBson == NULL)\n\
	{\n\
		dave_memset(unzip_data, 0x00, unzip_len);\n\
        return dave_false;\n\
	}\n\
    cpy_len = (size_t)unzip_len;\n\
    return t_bson_array_cpy_bin(pArrayBson, 0, (char *)unzip_data, &cpy_len);\n\
}\n"\


_uniondata_inc_head = "\
#ifndef _T_RPC_UNIONDATA_H__\n\
#define _T_RPC_UNIONDATA_H__\n\
#include \"dave_base.h\"\n"\


_uniondata_inc_end = "\
#endif\n\n"


def _creat_uniondata_include_file(file_id, include_list):
    file_id.write(_uniondata_src_private)
    file_id.write("\n// =====================================================================\n\n")
    return


def _creat_uniondata_zip_fun_file(file_id, union_name):
    file_id.write("void *\n")
    file_id.write("t_rpc_ver3_zip_"+union_name+"("+union_name+" *zip_data)\n")
    file_id.write("{\n\treturn _t_rpc_zip_uniondata((void *)zip_data, sizeof("+union_name+"));\n}\n\n")
    return


def _creat_uniondata_unzip_fun_file(file_id, union_name):
    file_id.write("dave_bool\nt_rpc_ver3_unzip_"+union_name+"("+union_name+" *unzip_data, void *pArrayBson)\n")
    file_id.write("{\n\treturn _t_rpc_unzip_uniondata((void *)unzip_data, sizeof("+union_name+"), pArrayBson);\n}\n\n")
    return


def _creat_uniondata_fun_file(file_id, union_table):
    for union_name in union_table:
        union_name = union_name.replace(" ", "")
        _creat_uniondata_zip_fun_file(file_id, union_name)
        _creat_uniondata_unzip_fun_file(file_id, union_name)
    return


def _creat_uniondata_src_file(union_table, include_list, file_name):
    with open(file_name, "w+", encoding="utf-8") as file_id:
        copyright_message(file_id)
        file_id.write(_uniondata_src_head)
        _creat_uniondata_include_file(file_id, include_list)
        _creat_uniondata_fun_file(file_id, union_table)
    return


def _creat_uniondata_inc_file(union_table, include_list, file_name):
    with open(file_name, "w+", encoding="utf-8") as file_id:
        copyright_message(file_id)
        file_id.write(_uniondata_inc_head)
        file_id.write("\n")
        for union_name in union_table:
            union_name = union_name.replace(" ", "")
            file_id.write("void * t_rpc_ver3_zip_"+union_name+"("+union_name+" *zip_data);\n")
            file_id.write("dave_bool t_rpc_ver3_unzip_"+union_name+"("+union_name+" *unzip_data, void *pArrayBson);\n\n")
        file_id.write(_uniondata_inc_end)
    return


def _creat_uniondata_remove_metadata(meta_table, union_table):
    for union_data in list(union_table):
        if meta_table.get(union_data, None) != None:
            del union_table[union_data]
    return


# =====================================================================


def creat_uniondata_file():
    union_table, include_list = find_union_table()
    meta_table = find_meta_table(ver3_metadata_src_file_name)
    _creat_uniondata_remove_metadata(meta_table, union_table)
    print(f"{len(union_table)}\tuniondata\twrite to {ver3_uniondata_src_file_name}")
    _creat_uniondata_src_file(union_table, include_list, ver3_uniondata_src_file_name)
    _creat_uniondata_inc_file(union_table, include_list, ver3_uniondata_inc_file_name)
    return
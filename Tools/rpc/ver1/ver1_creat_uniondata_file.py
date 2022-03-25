# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from rpc_cfg import *
from rpc_tools import *
from find.find_union_table import find_union_table
from find.find_meta_table import find_meta_table


_uniondata_src_head = "\
#include \"dave_macro.h\"\n\
#include \"dave_base.h\"\n\
#include \"dave_third_party.h\"\n\
#include \"tools_log.h\"\n"


_uniondata_src_private = "\
#define _t_rpc_zip_uniondata(zip_data, zip_len) __t_rpc_zip_uniondata__(zip_data, zip_len, (s8 *)__func__, (ub)__LINE__)\n\
static void *\n\
__t_rpc_zip_uniondata__(void *zip_data, ub zip_len, s8 *fun, ub line)\n\
{\n\
	void *pArrayJson = dave_json_array_malloc();\n\
    s8 *encodebase64_str;\n\
    ub encodebase64_len = zip_len * 4 + 1;\n\
\n\
    encodebase64_str = dave_malloc(encodebase64_len);\n\
    dave_base64_standard_encode((const u8 *)zip_data, zip_len, encodebase64_str, encodebase64_len);\n\
\n\
	dave_json_array_add_str(pArrayJson, encodebase64_str);\n\
\n\
    dave_free(encodebase64_str);\n\
\n\
	return pArrayJson;\n\
}\n\
\n\
#define _t_rpc_unzip_uniondata(unzip_data, unzip_len, pArrayJson) __t_rpc_unzip_uniondata__(unzip_data, unzip_len, pArrayJson, (s8 *)__func__, (ub)__LINE__)\n\
static dave_bool\n\
__t_rpc_unzip_uniondata__(void *unzip_data, ub unzip_len, void *pArrayJson, s8 *fun, ub line)\n\
{\n\
	void *pStrJson;\n\
    s8 *decodebase64_str;\n\
    ub decodebase64_len = unzip_len * 5;\n\
    ub decode_unzip_len;\n\
    dave_bool ret;\n\
\n\
	if(pArrayJson == NULL)\n\
	{\n\
		dave_memset(unzip_data, 0x00, unzip_len);\n\
        return dave_false;\n\
	}\n\
\n\
	pStrJson = dave_json_array_get_object(pArrayJson, 0);\n\
\n\
    decodebase64_str = dave_malloc(decodebase64_len);\n\
\n\
	ret = dave_json_get_str(pStrJson, NULL, decodebase64_str, &decodebase64_len);\n\
	if(ret == dave_true)\n\
    {\n\
        decode_unzip_len = unzip_len;\n\
        ret = dave_base64_standard_decode((const s8 *)decodebase64_str, decodebase64_len, (u8 *)unzip_data, &decode_unzip_len);\n\
        if(ret == dave_false)\n\
        {\n\
            TOOLSLOG(\"decode failed! <%s:%d>\", fun, line);\n\
        }\n\
        else\n\
        {\n\
            if(decode_unzip_len != unzip_len)\n\
            {\n\
                TOOLSLOG(\"unzip_len:%d/%d mismatch! <%s:%d>\", decode_unzip_len, unzip_len, fun, line);\n\
            }\n\
        }\n\
    }\n\
\n\
    if(ret == dave_false)\n\
    {\n\
	    dave_memset(unzip_data, 0x00, unzip_len);\n\
    }\n\
\n\
    dave_free(decodebase64_str);\n\
\n\
	return ret;\n\
}\n"\


_uniondata_inc_head = "\
#ifndef _T_RPC_UNIONDATA_H__\n\
#define _T_RPC_UNIONDATA_H__\n\
#include \"dave_macro.h\"\n"\


_uniondata_inc_end = "\
#endif\n\n"


def _creat_uniondata_include_content(file_id, include_list):
    include_message(file_id, include_list)
    return


def _creat_uniondata_include_file(file_id, include_list):
    _creat_uniondata_include_content(file_id, include_list)
    file_id.write(_uniondata_src_private)
    file_id.write("\n// =====================================================================\n\n")
    return


def _creat_uniondata_zip_fun_file(file_id, union_name):
    file_id.write("void *\n")
    file_id.write("t_rpc_ver1_zip_"+union_name+"("+union_name+" *zip_data)\n")
    file_id.write("{\n\treturn _t_rpc_zip_uniondata((void *)zip_data, sizeof("+union_name+"));\n}\n\n")
    return


def _creat_uniondata_unzip_fun_file(file_id, union_name):
    file_id.write("dave_bool\nt_rpc_ver1_unzip_"+union_name+"("+union_name+" *unzip_data, void *pArrayJson)\n")
    file_id.write("{\n\treturn _t_rpc_unzip_uniondata((void *)unzip_data, sizeof("+union_name+"), pArrayJson);\n}\n\n")
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
        _creat_uniondata_include_content(file_id, include_list)
        file_id.write("\n")
        for union_name in union_table:
            union_name = union_name.replace(" ", "")
            file_id.write("void * t_rpc_ver1_zip_"+union_name+"("+union_name+" *zip_data);\n")
            file_id.write("dave_bool t_rpc_ver1_unzip_"+union_name+"("+union_name+" *unzip_data, void *pArrayJson);\n\n")
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
    meta_table = find_meta_table(ver1_metadata_src_file_name)
    _creat_uniondata_remove_metadata(meta_table, union_table)
    print(f"{len(union_table)}\tuniondata\twrite to {ver1_uniondata_src_file_name}")
    _creat_uniondata_src_file(union_table, include_list, ver1_uniondata_src_file_name)
    _creat_uniondata_inc_file(union_table, include_list, ver1_uniondata_inc_file_name)
    return
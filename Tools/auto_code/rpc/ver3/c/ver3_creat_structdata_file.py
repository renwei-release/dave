# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from autocode_cfg import *
from autocode_tools import *
from find.find_other_struct_table import find_other_struct_table


_structdata_src_head = "\
#include \"dave_base.h\"\n\
#include \"dave_os.h\"\n\
#include \"dave_tools.h\"\n\
#include \"dave_3rdparty.h\"\n\
#include \"t_rpc_ver3_enumdata.h\"\n\
#include \"t_rpc_ver3_metadata.h\"\n\
#include \"t_rpc_ver3_structdata.h\"\n\
#include \"tools_log.h\"\n"


_structdata_inc_head = "\
#ifndef _T_RPC_STRUCTDATA_H__\n\
#define _T_RPC_STRUCTDATA_H__\n\
#include \"dave_base.h\"\n"\


_structdata_inc_end = "\
#endif\n\n"


def _struct_type_ptr_at_meta_table(struct_type, meta_table):
    type = struct_type + '_ptr'
    if type in meta_table:
        return True
    return False


def _creat_structdata_zip_fun_dimension_object(file_id, is_struct, struct_type, struct_name, struct_dimension):
    if is_struct == True:
        file_id.write("\n\tt_bson_add_object(pStructBson, \""+struct_type
            +"-"+struct_name+"\", t_rpc_ver3_zip_"+struct_type
            +"_d(zip_data->"+struct_name+", "+struct_dimension+"));")
    else:
        if struct_type == 's8' or struct_type == 'char':
            high_d, high_l = struct_dimension_decomposition(struct_dimension)
            file_id.write("\n\tt_bson_add_object(pStructBson, \""+struct_type
                +"-"+struct_name+"\", t_rpc_ver3_zip_"+struct_type
                +"_d(("+struct_type+" *)(zip_data->"+struct_name+"), "+high_d+", "+high_l+"));")
        else:
            file_id.write("\n\tt_bson_add_object(pStructBson, \""+struct_type
                +"-"+struct_name+"\", t_rpc_ver3_zip_"+struct_type
                +"_d(("+struct_type+" *)(zip_data->"+struct_name+"), "+struct_dimension+"));")       
    return


def _creat_structdata_zip_fun_object(file_id, struct_table, meta_table, struct_data):
    for struct_object in struct_data:
        struct_name = struct_object['n']
        struct_type = struct_object['t']
        struct_dimension = struct_object['d']
        has_ptr = struct_object['p']
        is_struct = struct_on_the_table(struct_type, struct_table)
        if struct_dimension == None:
            if is_struct == False:
                if has_ptr == False:
                    file_id.write("\n\tt_bson_add_object(pStructBson, \""+struct_type
                        +"-"+struct_name+"\", t_rpc_ver3_zip_"+struct_type
                        +"(zip_data->"+struct_name+"));")
                else:
                    file_id.write("\n\tt_bson_add_object(pStructBson, \""+struct_type
                        +"-"+struct_name+"\", t_rpc_ver3_zip_"+struct_type+"_ptr"
                        +"(zip_data->"+struct_name+"));")
            else:
                if has_ptr == False:
                    file_id.write("\n\tt_bson_add_object(pStructBson, \""+struct_type
                        +"-"+struct_name+"\", t_rpc_ver3_zip_"+struct_type
                        +"(&(zip_data->"+struct_name+")));")
                else:
                    if _struct_type_ptr_at_meta_table(struct_type, meta_table) == True:
                        file_id.write("\n\tt_bson_add_object(pStructBson, \""+struct_type
                            +"-"+struct_name+"\", t_rpc_ver3_zip_"+struct_type+"_ptr"
                            +"(zip_data->"+struct_name+"));")
                    else:
                        file_id.write("\n\tt_bson_add_object(pStructBson, \""+struct_type
                            +"-"+struct_name+"\", t_rpc_ver3_zip_"+struct_type
                            +"(zip_data->"+struct_name+"));")                        
        else:
            _creat_structdata_zip_fun_dimension_object(file_id, is_struct, struct_type, struct_name, struct_dimension)
    return


def _creat_structdata_unzip_fun_dimension_object(file_id, is_struct, struct_type, struct_name, struct_dimension):
    if is_struct == True:
        file_id.write("\t\tt_rpc_ver3_unzip_"+struct_type
            +"_d(unzip_data->"+struct_name+", "+struct_dimension+", t_bson_inq_object(pStructBson, \""
            +struct_type+"-"+struct_name+"\"));\n")
    else:
        if struct_type == 's8' or struct_type == 'char':
            high_d, high_l = struct_dimension_decomposition(struct_dimension)
            file_id.write("\t\tt_rpc_ver3_unzip_"+struct_type
                +"_d(("+struct_type+" *)(unzip_data->"+struct_name+"), "
                +high_d+", "+high_l+", t_bson_inq_object(pStructBson, \""
                +struct_type+"-"+struct_name+"\"));\n")
        else:
            file_id.write("\t\tt_rpc_ver3_unzip_"+struct_type
                +"_d(("+struct_type+" *)(unzip_data->"+struct_name+"), "
                +struct_dimension+", t_bson_inq_object(pStructBson, \""
                +struct_type+"-"+struct_name+"\"));\n")
    return


def _creat_structdata_unzip_fun_object(file_id, struct_table, meta_table, struct_data):
    for struct_object in struct_data:
        struct_name = struct_object['n']
        struct_type = struct_object['t']
        struct_dimension = struct_object['d']
        has_ptr = struct_object['p']
        is_struct = struct_on_the_table(struct_type, struct_table)
        if struct_dimension == None:
            if is_struct == False:
                if has_ptr == False:
                    file_id.write("\t\tt_rpc_ver3_unzip_"+struct_type
                        +"(&(unzip_data->"+struct_name+"), t_bson_inq_object(pStructBson, \""
                        +struct_type+"-"+struct_name+"\"));\n")
                else:
                    file_id.write("\t\tt_rpc_ver3_unzip_"+struct_type+"_ptr"
                        +"(&(unzip_data->"+struct_name+"), t_bson_inq_object(pStructBson, \""
                        +struct_type+"-"+struct_name+"\"));\n")
            else:
                if has_ptr == False:
                    file_id.write("\t\tt_rpc_ver3_unzip_"+struct_type
                        +"(&(unzip_data->"+struct_name+"), t_bson_inq_object(pStructBson, \""
                        +struct_type+"-"+struct_name+"\"));\n")
                else:
                    if _struct_type_ptr_at_meta_table(struct_type, meta_table) == True:
                        file_id.write("\t\tt_rpc_ver3_unzip_"+struct_type+"_ptr"
                            +"(&(unzip_data->"+struct_name+"), t_bson_inq_object(pStructBson, \""
                            +struct_type+"-"+struct_name+"\"));\n")
                    else:
                        file_id.write("\t\tt_rpc_ver3_unzip_"+struct_type
                            +"(unzip_data->"+struct_name+", t_bson_inq_object(pStructBson, \""
                            +struct_type+"-"+struct_name+"\"));\n")                        
        else:
            _creat_structdata_unzip_fun_dimension_object(file_id, is_struct, struct_type, struct_name, struct_dimension)
    return


def _creat_structdata_zip_fun_file(file_id, struct_table, meta_table, struct_name, struct_data):
    file_id.write("void *\nt_rpc_ver3_zip_"+struct_name+"("+struct_name+" *zip_data)\n")
    file_id.write("{\n\tvoid *pStructBson = t_bson_malloc_object();\n")
    _creat_structdata_zip_fun_object(file_id, struct_table, meta_table, struct_data)
    file_id.write("\n\n\treturn pStructBson;\n}\n\n")
    return


def _creat_structdata_unzip_fun_file(file_id, struct_table, meta_table, struct_name, struct_data):
    file_id.write("dave_bool\nt_rpc_ver3_unzip_"+struct_name+"("+struct_name+" *unzip_data, void *pStructBson)\n")
    file_id.write("{\n")
    file_id.write("	dave_bool ret;\n")
    file_id.write("\n")
    file_id.write("	if(pStructBson == NULL)\n")
    file_id.write("	{\n")
    file_id.write("		TOOLSLTRACE(360,1,\"the pBson is NULL on "+struct_name+"\");\n")
    file_id.write("		dave_memset(unzip_data, 0x00, sizeof("+struct_name+"));\n")
    file_id.write("		ret = dave_false;\n")
    file_id.write("	}\n")
    file_id.write("	else\n")
    file_id.write("	{\n")
    _creat_structdata_unzip_fun_object(file_id, struct_table, meta_table, struct_data)
    file_id.write("		ret = dave_true;\n")
    file_id.write("	}\n")
    file_id.write("\n")
    file_id.write("	return ret;\n")
    file_id.write("}\n\n")
    return


def _creat_structdata_zip_funs_file(file_id, struct_name):
    file_id.write("void *\nt_rpc_ver3_zip_"+struct_name+"_d("+struct_name+" *zip_data, ub zip_len)\n")
    file_id.write("{\n	void *pArrayBson = t_bson_malloc_array();\n")
    file_id.write("	ub zip_index;\n")
    file_id.write("\n")
    file_id.write("	for(zip_index=0; zip_index<zip_len; zip_index++)\n")
    file_id.write("	{\n")
    file_id.write("		t_bson_array_add_object(pArrayBson, t_rpc_ver3_zip_"+struct_name+"(&(zip_data[zip_index])));\n")
    file_id.write("	}\n")
    file_id.write("\n")
    file_id.write("	return pArrayBson;\n")
    file_id.write("}\n\n")
    return


def _creat_structdata_unzip_funs_file(file_id, struct_name):
    file_id.write("dave_bool\n")
    file_id.write("t_rpc_ver3_unzip_"+struct_name+"_d("+struct_name+" *unzip_data, ub unzip_len, void *pArrayBson)\n")
    file_id.write("{\n")
    file_id.write("	sb array_len, array_index;\n")
    file_id.write("\n")
    file_id.write("	dave_memset(unzip_data, 0x00, unzip_len * sizeof("+struct_name+"));\n")
    file_id.write("\n")
    file_id.write("	if(pArrayBson == NULL)\n")
    file_id.write("	{\n")
    file_id.write("		return dave_false;\n")
    file_id.write("	}\n")
    file_id.write("\n")
    file_id.write("	array_len = t_bson_array_number(pArrayBson);\n")
    file_id.write("	if(array_len > (sb)unzip_len)\n")
    file_id.write("	{\n")
    file_id.write("		array_len = (sb)unzip_len;\n")
    file_id.write("	}\n")
    file_id.write("\n")
    file_id.write("	for(array_index=0; array_index<array_len; array_index++)\n")
    file_id.write("	{\n")
    file_id.write("		t_rpc_ver3_unzip_"+struct_name+"(&unzip_data[array_index], t_bson_array_inq_object(pArrayBson, array_index));\n")
    file_id.write("	}\n")
    file_id.write("\n")
    file_id.write("	return dave_true;\n")
    file_id.write("}\n\n")
    return


def _creat_structdata_fun_file(file_id, struct_table, meta_table):
    for struct_name in struct_table.keys():
        struct_data = struct_table_get(struct_table[struct_name])
        if struct_data != None:
            _creat_structdata_zip_fun_file(file_id, struct_table, meta_table, struct_name, struct_data)
            _creat_structdata_unzip_fun_file(file_id, struct_table, meta_table, struct_name, struct_data)
            _creat_structdata_zip_funs_file(file_id, struct_name)
            _creat_structdata_unzip_funs_file(file_id, struct_name)
    return


def _creat_structdata_src_file(struct_table, meta_table, head_list, file_name):
    with open(file_name, "w+", encoding="utf-8") as file_id:
        copyright_message(file_id)
        file_id.write(_structdata_src_head)
        include_message(file_id, head_list)
        file_id.write("// =====================================================================\n\n")
        _creat_structdata_fun_file(file_id, struct_table, meta_table)
    return


def _creat_structdata_inc_file(struct_table, head_list, file_name):
    with open(file_name, "w+", encoding="utf-8") as file_id:
        copyright_message(file_id)
        file_id.write(_structdata_inc_head)
        include_message(file_id, head_list)
        for struct_name in struct_table.keys():
            struct_name = struct_name.replace(" ", "")
            file_id.write("void * t_rpc_ver3_zip_"+struct_name+"("+struct_name+" *zip_data);\n")
            file_id.write("dave_bool t_rpc_ver3_unzip_"+struct_name+"("+struct_name+" *unzip_data, void *pStructBson);\n\n")
            file_id.write("void * t_rpc_ver3_zip_"+struct_name+"_d("+struct_name+" *zip_data, ub zip_len);\n")
            file_id.write("dave_bool t_rpc_ver3_unzip_"+struct_name+"_d("+struct_name+" *unzip_data, ub unzip_len, void *pArrayBson);\n\n")
        file_id.write(_structdata_inc_end)
    return


# =====================================================================


def creat_structdata_file(param):
    struct_table = param['other_struct_table']
    meta_table = param['meta_table']
    head_list = param['other_include_list']

    print(f"{len(struct_table)}\tstructdata\twrite to {rpc_ver3_structdata_src_file_name}")
    _creat_structdata_src_file(struct_table, meta_table, head_list, rpc_ver3_structdata_src_file_name)
    _creat_structdata_inc_file(struct_table, head_list, rpc_ver3_structdata_inc_file_name)
    return
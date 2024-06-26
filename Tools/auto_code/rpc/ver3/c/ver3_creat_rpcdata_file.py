# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from autocode_cfg import *
from autocode_tools import *
from find.find_msg_struct_table import find_msg_struct_table
from find.find_msg_id_list import find_msg_id_list


_rpcdata_src_head = "\
#include \"dave_base.h\"\n\
#include \"dave_os.h\"\n\
#include \"dave_tools.h\"\n\
#include \"dave_3rdparty.h\"\n\
#include \"t_rpc_ver3_enumdata.h\"\n\
#include \"t_rpc_ver3_msgdata.h\"\n\
#include \"t_rpc_ver3_rpcdata.h\"\n\
#include \"t_rpc_ver3_structdata.h\"\n\
#include \"tools_log.h\"\n\n"


_rpcdata_inc_head = "\
#ifndef _T_RPC_RPCDATA_H__\n\
#define _T_RPC_RPCDATA_H__\n\
#include \"dave_base.h\"\n\n"\


_rpcdata_inc_end = "\
#endif\n\n"


def _creat_rpc_data_private_zip_file(file_id, msg_table):
    file_id.write("static inline void *\n_t_rpc_zip(ub msg_id, void *msg_body, ub msg_len)\n")
    file_id.write("{\n")
    file_id.write("	void *pBson;\n")
    file_id.write("\n")
    file_id.write("	switch((sb)msg_id)\n")
    file_id.write("	{\n")
    for msg_name in msg_table.keys():
        struct_name = msg_table[msg_name]
        file_id.write("\t\tcase "+msg_name+":\n")
        file_id.write("\t			pBson = t_rpc_ver3_zip_"+struct_name+"(("+struct_name+" *)msg_body, msg_len);\n")
        file_id.write("\t		break;\n")
    file_id.write("		default:\n")
    file_id.write("				TOOLSLOG(\"msg_id:%d zip failed!\", msg_id);\n")
    file_id.write(" 				pBson = NULL;\n")
    file_id.write("			break;\n")
    file_id.write("	}\n")
    file_id.write("\n\treturn pBson;\n}\n\n")


def _creat_rpc_data_private_unzip_file(file_id, msg_table):
    file_id.write("static inline dave_bool\n_t_rpc_unzip(void **msg_body, ub *msg_len, ub msg_id, void *pBson)\n")
    file_id.write("{\n\tdave_bool ret;\n\n")
    file_id.write("	switch((sb)msg_id)\n")
    file_id.write("	{\n")
    for msg_name in msg_table.keys():
        struct_name = msg_table[msg_name]
        file_id.write("\t\tcase "+msg_name+":\n")
        file_id.write("\t			ret = t_rpc_ver3_unzip_"+struct_name+"(msg_body, msg_len, pBson);\n")
        file_id.write("\t		break;\n")
    file_id.write("		default:\n")
    file_id.write(" 				TOOLSLOG(\"msg_id:%d unzip failed!\", msg_id);\n")
    file_id.write(" 				ret = dave_false;\n")
    file_id.write(" 			break;\n")
    file_id.write("	}\n")
    file_id.write("\n\treturn ret;\n}\n\n")


def _creat_rpc_data_private_ptr_file(file_id, msg_table):
    file_id.write("static inline void *\n_t_rpc_ptr(ub msg_id, void *msg_body, void *new_ptr)\n")
    file_id.write("{\n")
    file_id.write("	void *ptr;\n")
    file_id.write("\n")
    file_id.write("	switch((sb)msg_id)\n")
    file_id.write("	{\n")
    for msg_name in msg_table.keys():
        struct_name = msg_table[msg_name]
        file_id.write("\t\tcase "+msg_name+":\n")
        file_id.write("\t			ptr = t_rpc_ver3_ptr_"+struct_name+"(("+struct_name+" *)msg_body, new_ptr);\n")
        file_id.write("\t		break;\n")
    file_id.write("		default:\n")
    file_id.write("				TOOLSLOG(\"msg_id:%d zip failed!\", msg_id);\n")
    file_id.write(" 				ptr = NULL;\n")
    file_id.write("			break;\n")
    file_id.write("	}\n")
    file_id.write("\n\treturn ptr;\n}\n\n")


def _creat_rpc_data_msg_sizeof_file(file_id, msg_table):
    file_id.write("static inline ub\n_t_rpc_sizeof(ub msg_id)\n")
    file_id.write("{\n")
    file_id.write("	ub msg_len;\n")
    file_id.write("\n")
    file_id.write("	switch((sb)msg_id)\n")
    file_id.write("	{\n")
    for msg_name in msg_table.keys():
        struct_name = msg_table[msg_name]
        file_id.write("\t\tcase "+msg_name+":\n")
        file_id.write("\t			msg_len = t_rpc_ver3_sizeof_"+struct_name+"();\n")
        file_id.write("\t		break;\n")
    file_id.write("		default:\n")
    file_id.write("		        msg_len = 0;\n")
    file_id.write("			break;\n")
    file_id.write("	}\n")
    file_id.write("\n\treturn msg_len;\n}\n\n")


def _creat_rpcdata_zip_file(file_id):
    file_id.write("void *\nt_rpc_ver3_zip(void *pChainBson, void *pRouterBson, ub msg_id, void *msg_body, ub msg_len)\n")
    file_id.write("{\n")
    file_id.write("	void *pBson;\n")
    file_id.write("\n\tpBson = _t_rpc_zip(msg_id, msg_body, msg_len);\n")
    file_id.write("	if(pBson == NULL)\n")
    file_id.write("	{\n")
    file_id.write("\t\tTOOLSLOG(\"msg_id:%d zip failed!\", msg_id);\n")
    file_id.write("		return NULL;\n\t}\n")
    file_id.write("\n\tt_bson_add_int(pBson, \"rpc_version\", 3);\n")
    file_id.write("\n\t#ifdef LEVEL_PRODUCT_alpha")
    file_id.write("\n\tt_bson_add_int64(pBson, \"rpc_time\", (u64)dave_os_time_us());")
    file_id.write("\n\t#endif\n")
    file_id.write("\n\tif(pChainBson != NULL)\n")
    file_id.write("\t\tt_bson_add_object(pBson, \"chain\", pChainBson);\n")
    file_id.write("\n\tif(pRouterBson != NULL)\n")
    file_id.write("\t\tt_bson_add_object(pBson, \"router\", pRouterBson);\n")
    file_id.write("\n\treturn pBson;\n")
    file_id.write("}\n")
    return


def _creat_rpcdata_unzip_file(file_id):
    file_id.write("dave_bool\nt_rpc_ver3_unzip(s8 *data_from, void **ppChainBson, void **ppRouterBson, void **msg_body, ub *msg_len, ub msg_id, s8 *packet_ptr, ub packet_len)\n")
    file_id.write("{\r\tvoid *pBson;\n")
    file_id.write("\tdave_bool ret = dave_false;\n")
    file_id.write("\n")
    file_id.write("\tpBson = t_serialize_to_bson((char *)packet_ptr, (int)packet_len);\n")
    file_id.write("\tif(pBson == NULL)\n")
    file_id.write("\t{\n")
    file_id.write("\t	TOOLSLOG(\"msg_id:%d has invalid bson:%d\", msg_id, packet_len);\n")
    file_id.write("\t	return dave_false;\n")
    file_id.write("\t}\n")
    file_id.write("\n\t#ifdef LEVEL_PRODUCT_alpha")
    file_id.write("\n\ts64 rpc_time;")
    file_id.write("\n\tif(t_bson_inq_int64(pBson, \"rpc_time\", (u64 *)(&rpc_time)) == true) {")
    file_id.write("\n\t\trpc_time = (s64)dave_os_time_us() - rpc_time;")
    file_id.write("\n\t\tif(rpc_time > 30*1000000)")
    file_id.write("\n\t\t\tTOOLSLTRACE(60,1,\"from:%s msg_id:%s took too much time:%lds in transit or the time of the transmission parties is out of sync.\", data_from, msgstr(msg_id), rpc_time/1000000);")
    file_id.write("\n\t}")
    file_id.write("\n\t#endif\n")
    file_id.write("\n\tret = _t_rpc_unzip(msg_body, msg_len, msg_id, pBson);\n")
    file_id.write("\n\t*ppChainBson = t_bson_clone_object(pBson, \"chain\");\n")
    file_id.write("\n\t*ppRouterBson = t_bson_clone_object(pBson, \"router\");\n")
    file_id.write("\n\tt_bson_free_object(pBson);\n")
    file_id.write("\n")
    file_id.write("\treturn ret;\n")
    file_id.write("}\n")
    return


def _creat_rpcdata_ptr_file(file_id):
    file_id.write("void *\nt_rpc_ver3_ptr(ub msg_id, void *msg_body, void *new_ptr)\n")
    file_id.write("{\n")
    file_id.write("\treturn _t_rpc_ptr(msg_id, msg_body, new_ptr);\n")
    file_id.write("}\n")
    return


def _creat_rpcdata_sizeof_file(file_id):
    file_id.write("ub\nt_rpc_ver3_sizeof(ub msg_id)\n")
    file_id.write("{\n")
    file_id.write("\treturn _t_rpc_sizeof(msg_id);\n")
    file_id.write("}\n")
    return


def _creat_rpcdata_src_file(msg_table, file_name):
    with open(file_name, "w+", encoding="utf-8") as file_id:
        copyright_message(file_id)
        file_id.write(_rpcdata_src_head)
        _creat_rpc_data_private_zip_file(file_id, msg_table)
        _creat_rpc_data_private_unzip_file(file_id, msg_table)
        _creat_rpc_data_private_ptr_file(file_id, msg_table)
        _creat_rpc_data_msg_sizeof_file(file_id, msg_table)
        file_id.write("// =====================================================================\n\n")
        _creat_rpcdata_zip_file(file_id)
        file_id.write("\n")
        _creat_rpcdata_unzip_file(file_id)
        file_id.write("\n")
        _creat_rpcdata_ptr_file(file_id)
        file_id.write("\n")
        _creat_rpcdata_sizeof_file(file_id)
        file_id.write("\n")
    return


def _creat_rpcdata_inc_file(file_name):
    with open(file_name, "w+", encoding="utf-8") as file_id:
        copyright_message(file_id)
        file_id.write(_rpcdata_inc_head)
        file_id.write("void * t_rpc_ver3_zip(void *pChainBson, void *pRouterBson, ub msg_id, void *msg_body, ub msg_len);\n")
        file_id.write("dave_bool t_rpc_ver3_unzip(s8 *data_from, void **ppChainBson, void **ppRouterBson, void **msg_body, ub *msg_len, ub msg_id, s8 *packet_ptr, ub packet_len);\n")
        file_id.write("void * t_rpc_ver3_ptr(ub msg_id, void *msg_body, void *new_ptr);\n")
        file_id.write("ub t_rpc_ver3_sizeof(ub msg_id);\n\n")
        file_id.write(_rpcdata_inc_end)
    return


def _check_msg_table_on_the_msg_id_list(msg_table, msg_id_list):
    remove_msg_id_table = []
    for table_msg_id, table_msg_struct in msg_table.items():
        table_msg_id_in_msg_list = False
        for msg_list_id in msg_id_list:
            if msg_list_id == table_msg_id:
                table_msg_id_in_msg_list = True
                break
        if table_msg_id_in_msg_list == False:
            remove_msg_id_table.append(table_msg_id)
    for remove_msg_id in remove_msg_id_table:
        print(f"\033[0;31;40mRemove message {remove_msg_id}, it is not defined correctly.\033[0m")
        msg_table.pop(remove_msg_id)
    return msg_table


# =====================================================================


def creat_rpcdata_file(param):
    file_list = param['file_list']
    msg_table = param['msg_name_table']
    struct_table = param['msg_struct_table']
    include_list = param['msg_include_list']

    msg_id_list = find_msg_id_list(file_list)
    msg_table = _check_msg_table_on_the_msg_id_list(msg_table, msg_id_list)
    print(f"{len(struct_table)}\trpcdata\t\twrite to {rpc_ver3_rpcdata_src_file_name}")
    _creat_rpcdata_src_file(msg_table, rpc_ver3_rpcdata_src_file_name)
    _creat_rpcdata_inc_file(rpc_ver3_rpcdata_inc_file_name)
    return
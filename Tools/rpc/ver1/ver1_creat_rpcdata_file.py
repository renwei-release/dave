# -*- coding: utf-8 -*-

#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.03.11.
#

from rpc_cfg import *
from rpc_tools import *
from find.find_msg_struct_table import find_msg_struct_table
from find.find_msg_id_list import find_msg_id_list


_rpcdata_src_head = "\
#include \"dave_macro.h\"\n\
#include \"dave_base.h\"\n\
#include \"dave_third_party.h\"\n\
#include \"t_rpc_ver1_enumdata.h\"\n\
#include \"t_rpc_ver1_fundata.h\"\n\
#include \"t_rpc_ver1_msgdata.h\"\n\
#include \"t_rpc_ver1_rpcdata.h\"\n\
#include \"t_rpc_ver1_structdata.h\"\n\
#include \"t_rpc_ver1_uniondata.h\"\n\
#include \"tools_log.h\"\n"


_rpcdata_inc_head = "\
#ifndef _T_RPC_RPCDATA_H__\n\
#define _T_RPC_RPCDATA_H__\n\
#include \"dave_macro.h\"\n\n"\


_rpcdata_inc_end = "\
#endif\n\n"


def _creat_rpc_data_private_zip_file(file_id, msg_table):
    file_id.write("static void *\n_t_rpc_zip(ub msg_id, void *msg_body, ub msg_len)\n")
    file_id.write("{\n")
    file_id.write("	void *pJson;\n")
    file_id.write("\n")
    file_id.write("	switch((sb)msg_id)\n")
    file_id.write("	{\n")
    for msg_name in msg_table.keys():
        struct_name = msg_table[msg_name]
        file_id.write("\t\tcase "+msg_name+":\n")
        file_id.write("\t			pJson = t_rpc_ver1_zip_"+struct_name+"(("+struct_name+" *)msg_body, msg_len);\n")
        file_id.write("\t		break;\n")
    file_id.write("		default:\n")
    file_id.write("				pJson = NULL;\n")
    file_id.write("			break;\n")
    file_id.write("	}\n")
    file_id.write("\n\treturn pJson;\n}\n\n")


def _creat_rpc_data_private_unzip_file(file_id, msg_table):
    file_id.write("static dave_bool\n_t_rpc_unzip(void **msg_body, ub *msg_len, ub msg_id, void *pJson)\n")
    file_id.write("{\n\tdave_bool ret;\n\n")
    file_id.write("	switch((sb)msg_id)\n")
    file_id.write("	{\n")
    for msg_name in msg_table.keys():
        struct_name = msg_table[msg_name]
        file_id.write("\t\tcase "+msg_name+":\n")
        file_id.write("\t			ret = t_rpc_ver1_unzip_"+struct_name+"(msg_body, msg_len, pJson);\n")
        file_id.write("\t		break;\n")
    file_id.write("		default:\n")
    file_id.write("				ret = dave_false;\n")
    file_id.write("			break;\n")
    file_id.write("	}\n")
    file_id.write("\n\treturn ret;\n}\n\n")


def _creat_rpcdata_zip_file(file_id):
    file_id.write("void *\nt_rpc_ver1_zip(THREADMSGID msg_id, void *msg_body, ub msg_len)\n")
    file_id.write("{\n")
    file_id.write("	void *pJson;\n")
    file_id.write("\n\tpJson = _t_rpc_zip(msg_id, msg_body, msg_len);\n")
    file_id.write("	if(pJson == NULL)\n")
    file_id.write("	{\n")
    file_id.write("\t\tTOOLSLOG(\"msg_id:%d zip failed!\", msg_id);\n")
    file_id.write("		return NULL;\n\t}\n")
    file_id.write("\n\tdave_json_add_ub(pJson, \"rpc_version\", 1);\n")
    file_id.write("\n\treturn pJson;\n")
    file_id.write("}\n")
    return


def _creat_rpcdata_unzip_file(file_id):
    file_id.write("dave_bool\nt_rpc_ver1_unzip(void **msg_body, ub *msg_len, ub msg_id, s8 *packet_ptr, ub packet_len)\n")
    file_id.write("{\r\tvoid *pJson;\n")
    file_id.write("\tdave_bool ret = dave_false;\n")
    file_id.write("\n")
    file_id.write("\tpJson = dave_string_to_json(packet_ptr, packet_len);\n")
    file_id.write("\tif(pJson == NULL)\n")
    file_id.write("\t{\n")
    file_id.write("\t	TOOLSLOG(\"msg_id:%d has invalid json:%d/%s\", msg_id, packet_len, packet_ptr);\n")
    file_id.write("\t	return dave_false;\n")
    file_id.write("\t}\n")
    file_id.write("\n\tret = _t_rpc_unzip(msg_body, msg_len, msg_id, pJson);\n")
    file_id.write("\n\tdave_json_free(pJson);\n")
    file_id.write("\n")
    file_id.write("\treturn ret;\n")
    file_id.write("}\n\n")
    return


def _creat_rpcdata_src_file(msg_table, file_list, file_name):
    with open(file_name, "w+", encoding="utf-8") as file_id:
        copyright_message(file_id)
        file_id.write(_rpcdata_src_head)
        include_message(file_id, file_list, file_name)
        _creat_rpc_data_private_zip_file(file_id, msg_table)
        _creat_rpc_data_private_unzip_file(file_id, msg_table)
        file_id.write("// =====================================================================\n\n")
        _creat_rpcdata_zip_file(file_id)
        file_id.write("\n")
        _creat_rpcdata_unzip_file(file_id)
    return


def _creat_rpcdata_inc_file(file_list, file_name):
    with open(file_name, "w+", encoding="utf-8") as file_id:
        copyright_message(file_id)
        file_id.write(_rpcdata_inc_head)
        file_id.write("void * t_rpc_ver1_zip(ub msg_id, void *msg_body, ub msg_len);\n")
        file_id.write("dave_bool t_rpc_ver1_unzip(void **msg_body, ub *msg_len, ub msg_id, s8 *packet_ptr, ub packet_len);\n\n")
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


def creat_rpcdata_file():
    msg_table, struct_table, file_list = find_msg_struct_table()
    msg_id_list = find_msg_id_list()
    msg_table = _check_msg_table_on_the_msg_id_list(msg_table, msg_id_list)
    print(f"{len(struct_table)}\trpcdata\t\twrite to {ver1_rpcdata_src_file_name}")
    _creat_rpcdata_src_file(msg_table, file_list, ver1_rpcdata_src_file_name)
    _creat_rpcdata_inc_file(file_list, ver1_rpcdata_inc_file_name)
    return
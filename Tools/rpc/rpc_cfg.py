# -*- coding: utf-8 -*-

#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.03.11.
#
import os


rpc_detected_path_list = [
    "../../C/project",
]


rpc_disable_path_list = [
    "../../C/project/dave/os",
    "../../C/project/dave/third_party",
    "../../C/project/dave/tools",
    "../../C/project/dave/verno"
]


ver1_auto_dir = os.path.abspath(__file__).rsplit('/', 1)[0] + '/../../C/project/dave/tools/src/t_rpc/ver1/auto'
if not os.path.exists(ver1_auto_dir):
    os.makedirs(ver1_auto_dir)
ver1_metadata_src_file_name = "../../C/project/dave/tools/src/t_rpc/ver1/t_rpc_ver1_metadata.c"
ver1_metadata_inc_file_name = "../../C/project/dave/tools/src/t_rpc/ver1/t_rpc_ver1_metadata.h"
ver1_enumdata_src_file_name = ver1_auto_dir+"/t_rpc_ver1_enumdata.c"
ver1_enumdata_inc_file_name = ver1_auto_dir+"/t_rpc_ver1_enumdata.h"
ver1_uniondata_src_file_name = ver1_auto_dir+"/t_rpc_ver1_uniondata.c"
ver1_uniondata_inc_file_name = ver1_auto_dir+"/t_rpc_ver1_uniondata.h"
ver1_fundata_src_file_name = ver1_auto_dir+"/t_rpc_ver1_fundata.c"
ver1_fundata_inc_file_name = ver1_auto_dir+"/t_rpc_ver1_fundata.h"
ver1_structdata_src_file_name = ver1_auto_dir+"/t_rpc_ver1_structdata.c"
ver1_structdata_inc_file_name = ver1_auto_dir+"/t_rpc_ver1_structdata.h"
ver1_msgdata_src_file_name = ver1_auto_dir+"/t_rpc_ver1_msgdata.c"
ver1_msgdata_inc_file_name = ver1_auto_dir+"/t_rpc_ver1_msgdata.h"
ver1_rpcdata_src_file_name = ver1_auto_dir+"/t_rpc_ver1_rpcdata.c"
ver1_rpcdata_inc_file_name = ver1_auto_dir+"/t_rpc_ver1_rpcdata.h"


ver2_auto_dir = os.path.abspath(__file__).rsplit('/', 1)[0] + '/../../C/project/dave/tools/src/t_rpc/ver2/auto'
if not os.path.exists(ver2_auto_dir):
    os.makedirs(ver2_auto_dir)
ver2_proto_file_name = ver2_auto_dir+"/t_rpc_ver2_proto.proto"


ver3_auto_dir = os.path.abspath(__file__).rsplit('/', 1)[0] + '/../../C/project/dave/tools/src/t_rpc/ver3/auto'
if not os.path.exists(ver3_auto_dir):
    os.makedirs(ver3_auto_dir)
ver3_rpcinc_file_name = "../../C/project/dave/tools/inc/t_rpc.h"
ver3_metadata_src_file_name = "../../C/project/dave/tools/src/t_rpc/ver3/t_rpc_ver3_metadata.c"
ver3_metadata_inc_file_name = "../../C/project/dave/tools/src/t_rpc/ver3/t_rpc_ver3_metadata.h"
ver3_enumdata_src_file_name = ver3_auto_dir+"/t_rpc_ver3_enumdata.c"
ver3_enumdata_inc_file_name = ver3_auto_dir+"/t_rpc_ver3_enumdata.h"
ver3_uniondata_src_file_name = ver3_auto_dir+"/t_rpc_ver3_uniondata.c"
ver3_uniondata_inc_file_name = ver3_auto_dir+"/t_rpc_ver3_uniondata.h"
ver3_structdata_src_file_name = ver3_auto_dir+"/t_rpc_ver3_structdata.c"
ver3_structdata_inc_file_name = ver3_auto_dir+"/t_rpc_ver3_structdata.h"
ver3_msgdata_src_file_name = ver3_auto_dir+"/t_rpc_ver3_msgdata.c"
ver3_msgdata_inc_file_name = ver3_auto_dir+"/t_rpc_ver3_msgdata.h"
ver3_rpcdata_src_file_name = ver3_auto_dir+"/t_rpc_ver3_rpcdata.c"
ver3_rpcdata_inc_file_name = ver3_auto_dir+"/t_rpc_ver3_rpcdata.h"
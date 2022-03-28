# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
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
ver2_proto_file_name = ver2_auto_dir+"/t_rpc_ver2_proto.proto"


ver3_auto_dir = os.path.abspath(__file__).rsplit('/', 1)[0] + '/../../C/project/dave/tools/src/t_rpc/ver3/auto'
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

ver3_gomsgid_file_name = "../../Go/project/dave/base/dave_msg_id.go"
ver3_godefine_file_name = "../../Go/project/dave/base/dave_define.go"
ver3_gostruct_file_name = "../../Go/project/dave/base/dave_struct.go"
ver3_gomsgstruct_file_name = "../../Go/project/dave/base/dave_msg_struct.go"
ver3_goenum_file_name = "../../Go/project/dave/base/dave_enum.go"

ver3_pymsgid_file_name = "../../Python/project/dave/base/dave_msg_id.py"
ver3_pydefine_file_name = "../../Python/project/dave/base/dave_define.py"
ver3_pystruct_file_name = "../../Python/project/dave/base/dave_struct.py"
ver3_pymsgstruct_file_name = "../../Python/project/dave/base/dave_msg_struct.py"
ver3_pyenum_file_name = "../../Python/project/dave/base/dave_enum.py"
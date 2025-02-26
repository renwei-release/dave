# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import os


project_path_list = [
    "../../Project/C/project",
]


forgettable_path_list = [
    "../../Project/C/project/public/os",
    "../../Project/C/project/public/3rdparty",
    "../../Project/C/project/public/tools/inc/t_rpc.h",
    "../../Project/C/project/public/tools/src",
    "../../Project/C/project/public/verno",
	"../../Project/C/project/inc/third_party",
	"../../Project/C/project/src/verno",
    "../../Project/C/project/product/io/sip/src/osip",
    "../../Project/C/project/product/bdata/src/recorder/aliyun/src/aliyun"
]


rpc_hash_file = os.getcwd()+"/__RPC_FILE_HASH__"


rpc_ver2_auto_dir = os.path.abspath(__file__).rsplit('/', 1)[0] + '/../../Project/C/project/public/tools/src/t_rpc/ver2/auto'
rpc_ver2_proto_file_name = rpc_ver2_auto_dir+"/t_rpc_ver2_proto.proto"


rpc_ver3_auto_dir = os.path.abspath(__file__).rsplit('/', 1)[0] + '/../../Project/C/project/public/tools/src/t_rpc/ver3/auto'
rpc_ver3_rpcinc_file_name = "../../Project/C/project/public/tools/inc/t_rpc.h"
rpc_ver3_metadata_src_file_name = "../../Project/C/project/public/tools/src/t_rpc/ver3/t_rpc_ver3_metadata.c"
rpc_ver3_metadata_inc_file_name = "../../Project/C/project/public/tools/src/t_rpc/ver3/t_rpc_ver3_metadata.h"
rpc_ver3_enumdata_src_file_name = rpc_ver3_auto_dir+"/t_rpc_ver3_enumdata.c"
rpc_ver3_enumdata_inc_file_name = rpc_ver3_auto_dir+"/t_rpc_ver3_enumdata.h"
rpc_ver3_structdata_src_file_name = rpc_ver3_auto_dir+"/t_rpc_ver3_structdata.c"
rpc_ver3_structdata_inc_file_name = rpc_ver3_auto_dir+"/t_rpc_ver3_structdata.h"
rpc_ver3_msgdata_src_file_name = rpc_ver3_auto_dir+"/t_rpc_ver3_msgdata.c"
rpc_ver3_msgdata_inc_file_name = rpc_ver3_auto_dir+"/t_rpc_ver3_msgdata.h"
rpc_ver3_rpcdata_src_file_name = rpc_ver3_auto_dir+"/t_rpc_ver3_rpcdata.c"
rpc_ver3_rpcdata_inc_file_name = rpc_ver3_auto_dir+"/t_rpc_ver3_rpcdata.h"
rpc_ver3_fundata_src_file_name = rpc_ver3_auto_dir+"/t_rpc_ver3_fundata.c"
rpc_ver3_fundata_inc_file_name = rpc_ver3_auto_dir+"/t_rpc_ver3_fundata.h"


rpc_ver3_gomsgid_file_name = "../../Project/Go/project/public/auto/dave_msg_id.go"
rpc_ver3_godefine_file_name = "../../Project/Go/project/public/auto/dave_define.go"
rpc_ver3_gostruct_file_name = "../../Project/Go/project/public/auto/dave_struct.go"
rpc_ver3_gomsgstruct_file_name = "../../Project/Go/project/public/auto/dave_msg_struct.go"
rpc_ver3_goenum_file_name = "../../Project/Go/project/public/auto/dave_enum.go"


rpc_ver3_pymsgid_file_name = "../../Project/Python/project/public/auto/dave_msg_id.py"
rpc_ver3_pydefine_file_name = "../../Project/Python/project/public/auto/dave_define.py"
rpc_ver3_pystruct_file_name = "../../Project/Python/project/public/auto/dave_struct.py"
rpc_ver3_pymsgstruct_file_name = "../../Project/Python/project/public/auto/dave_msg_struct.py"
rpc_ver3_pyenum_file_name = "../../Project/Python/project/public/auto/dave_enum.py"


c_enumstr_src_file_name = "../../Project/C/project/public/tools/src/t_auto/t_auto_enumstr.c"
c_enumstr_inc_file_name = "../../Project/C/project/public/tools/src/t_auto/t_auto_enumstr.h"
go_enumstr_src_file_name = "../../Project/Go/project/public/auto/dave_enumstr.go"
py_enumstr_src_file_name = "../../Project/Python/project/public/auto/dave_enumstr.py"
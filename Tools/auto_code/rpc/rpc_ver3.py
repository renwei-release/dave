# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import os
from autocode_cfg import *
from rpc.ver3.creat_c import creat_ver3_c
from rpc.ver3.creat_go import creat_ver3_go
from rpc.ver3.creat_python import creat_ver3_python


def rpc_ver3(param):
    if not os.path.exists(rpc_ver3_auto_dir):
        os.makedirs(rpc_ver3_auto_dir)

    msg_id_table, fun_table = creat_ver3_c(param)

    param['msg_id_table'] = msg_id_table
    param['fun_table'] = fun_table

    creat_ver3_go(param)
    creat_ver3_python(param)
    return
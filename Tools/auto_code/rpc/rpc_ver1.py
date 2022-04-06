# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import os
from autocode_cfg import *
from rpc.ver1.ver1_creat_uniondata_file import creat_uniondata_file
from rpc.ver1.ver1_creat_enumdata_file import creat_enumdata_file
from rpc.ver1.ver1_creat_fundata_file import creat_fundata_file
from rpc.ver1.ver1_creat_structdata_file import creat_structdata_file
from rpc.ver1.ver1_creat_msgdata_file import creat_msgdata_file
from rpc.ver1.ver1_creat_rpcdata_file import creat_rpcdata_file


def rpc_ver1():
    if not os.path.exists(rpc_ver1_auto_dir):
        os.makedirs(rpc_ver1_auto_dir)
    creat_enumdata_file()
    creat_uniondata_file()
    creat_fundata_file()
    creat_structdata_file()
    creat_msgdata_file()
    creat_rpcdata_file()
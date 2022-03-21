# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2022 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2022.01.19.
#
import os
from rpc_cfg import *
from ver1.ver1_creat_uniondata_file import creat_uniondata_file
from ver1.ver1_creat_enumdata_file import creat_enumdata_file
from ver1.ver1_creat_fundata_file import creat_fundata_file
from ver1.ver1_creat_structdata_file import creat_structdata_file
from ver1.ver1_creat_msgdata_file import creat_msgdata_file
from ver1.ver1_creat_rpcdata_file import creat_rpcdata_file


def rpc_ver1():
    if not os.path.exists(ver1_auto_dir):
        os.makedirs(ver1_auto_dir)
    creat_enumdata_file()
    creat_uniondata_file()
    creat_fundata_file()
    creat_structdata_file()
    creat_msgdata_file()
    creat_rpcdata_file()
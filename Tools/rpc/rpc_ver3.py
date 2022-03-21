# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2022 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2022.02.06.
#

import os
from rpc_cfg import *
from ver3.ver3_creat_uniondata_file import creat_uniondata_file
from ver3.ver3_creat_enumdata_file import creat_enumdata_file
from ver3.ver3_creat_structdata_file import creat_structdata_file
from ver3.ver3_creat_msgdata_file import creat_msgdata_file
from ver3.ver3_creat_rpcdata_file import creat_rpcdata_file
from ver3.ver3_creat_rpcinc_file import creat_rpcinc_file


def rpc_ver3():
    if not os.path.exists(ver3_auto_dir):
        os.makedirs(ver3_auto_dir)

    creat_enumdata_file()
    creat_uniondata_file()
    creat_structdata_file()
    creat_msgdata_file()
    creat_rpcdata_file()
    creat_rpcinc_file()
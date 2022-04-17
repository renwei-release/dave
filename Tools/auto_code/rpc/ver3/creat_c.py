# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from rpc.ver3.c.ver3_creat_enumdata_file import creat_enumdata_file
from rpc.ver3.c.ver3_creat_structdata_file import creat_structdata_file
from rpc.ver3.c.ver3_creat_fundata_file import creat_fundata_file
from rpc.ver3.c.ver3_creat_msgdata_file import creat_msgdata_file
from rpc.ver3.c.ver3_creat_rpcdata_file import creat_rpcdata_file
from rpc.ver3.c.ver3_creat_rpcinc_file import creat_rpcinc_file


def creat_ver3_c(param):
    creat_msgdata_file(param)
    creat_structdata_file(param)
    fun_table = creat_fundata_file(param)
    creat_enumdata_file(param)
    creat_rpcdata_file(param)
    msg_id_table = creat_rpcinc_file(param)
    return msg_id_table, fun_table
# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import os
from rpc_cfg import *
from ver3.creat_c import creat_ver3_c
from ver3.creat_go import creat_ver3_go


def rpc_ver3():
    if not os.path.exists(ver3_auto_dir):
        os.makedirs(ver3_auto_dir)

    struct_table, msg_struct_table, enum_table, msg_id_table = creat_ver3_c()
    creat_ver3_go(struct_table, msg_struct_table, msg_id_table, enum_table)
# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from ver3.c.ver3_creat_uniondata_file import creat_uniondata_file
from ver3.c.ver3_creat_enumdata_file import creat_enumdata_file
from ver3.c.ver3_creat_structdata_file import creat_structdata_file
from ver3.c.ver3_creat_msgdata_file import creat_msgdata_file
from ver3.c.ver3_creat_rpcdata_file import creat_rpcdata_file
from ver3.c.ver3_creat_rpcinc_file import creat_rpcinc_file


def creat_ver3_c():
    creat_uniondata_file()
    struct_table = creat_structdata_file()
    msg_struct_table = creat_msgdata_file()
    enum_table = creat_enumdata_file()
    creat_rpcdata_file()
    msg_id_table = creat_rpcinc_file()
    return struct_table, msg_struct_table, enum_table, msg_id_table
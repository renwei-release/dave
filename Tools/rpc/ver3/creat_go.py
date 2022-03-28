# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from ver3.go.ver3_creat_gorpc_file import creat_gorpc_file


def creat_ver3_go(struct_table, msg_struct_table, msg_id_table, enum_table):
    return creat_gorpc_file(struct_table, msg_struct_table, msg_id_table, enum_table)
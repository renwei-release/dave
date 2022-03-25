# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from .find_other_struct_table import find_other_struct_table
from .find_msg_struct_table import find_msg_struct_table


# =====================================================================


def find_all_struct_table():
    _, msg_struct_table, _ = find_msg_struct_table()
    other_struct_table, _ = find_other_struct_table()
    struct_table = msg_struct_table
    struct_table.update(other_struct_table)
    return struct_table
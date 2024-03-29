# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import re
import traceback
from autocode_tools import *
from autocode_cfg import *


def _find_meta_list_from_file(file_name):
    meta_table = {}
    with open(file_name, "r", encoding="utf-8") as file_id:
        try:
            file_content = file_id.read()
        except:
            print(f"file_name:{file_name}")
            traceback.print_exc()
            return meta_table
        try:
            file_content = remove_annotation_data(file_content)
            result = re.findall("__t_rpc_ver[0-9]_zip_(.*?)__\(.*?", file_content)
            if result:
                for meta in result:
                    meta_table[meta] = meta
        except:
            print(f"file_name:{file_name}")
            traceback.print_exc()
            return meta_table
    return meta_table


# =====================================================================


def find_meta_table():
    meta_table = _find_meta_list_from_file(rpc_ver3_metadata_src_file_name)
    return list(set(meta_table))
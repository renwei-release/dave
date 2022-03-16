# -*- coding: utf-8 -*-

#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.03.11.
#

import re
import traceback
from rpc_tools import *

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
            result = re.findall(".*?t_rpc_zip_(.*?)[_,__,___,\(].*?\)", file_content)
            if result:
                for meta in result:
                    meta_table[meta] = meta
        except:
            print(f"file_name:{file_name}")
            traceback.print_exc()
            return meta_table      
    return meta_table


# =====================================================================


def find_meta_table(metadata_src_file_name):
    return _find_meta_list_from_file(metadata_src_file_name)
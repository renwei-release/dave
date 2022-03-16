# -*- coding: utf-8 -*-

#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.08.23.
#

import re
import traceback
from .find_msg_file_list import find_msg_file_list
from rpc_tools import *


# =====================================================================


def find_msg_id_list():
    msg_id_list = []
    file_list = find_msg_file_list()
    for file_name in file_list:
        with open(file_name, encoding="utf-8") as file_id:
            try:
                file_content = file_id.read()
            except:
                print(f"file_name:{file_name}")
                traceback.print_exc()
                return msg_id_list
            try:
                result = re.findall("\/\* for *(.+?) *message *\*\/", file_content)
                if result:
                    msg_id_list.extend(result)
            except:
                print(f"file_name:{file_name}")
                traceback.print_exc()
    return msg_id_list
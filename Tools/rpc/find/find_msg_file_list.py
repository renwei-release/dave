# -*- coding: utf-8 -*-

#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.03.11.
#

import re
import traceback
from .find_file_list import *


# =====================================================================


def find_msg_file_list():
    head_list = []
    file_list = find_file_list()
    for file_name in file_list:
        with open(file_name, encoding="utf-8") as file_id:
            try:
                file_content = file_id.read()
            except:
                print(f"file_name:{file_name}")
                traceback.print_exc()
                return head_list
            try:
                result = re.findall("\/\* for *(.+?) *message *\*\/", file_content)
                if result:
                    head_list.append(file_name)
            except:
                print(f"file_name:{file_name}")
                traceback.print_exc()
                return head_list
    return head_list
# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import re
import traceback
from .find_file_list import *


# =====================================================================


def find_msg_file_list(file_list=None):
    head_list = []
    for file_name in file_list:
        with open(file_name, encoding="utf-8") as file_id:
            try:
                file_content = file_id.read()
            except:
                print(f"1 find_msg_file_list file_name:{file_name}")
                return head_list
            try:
                result = re.findall("\/\* for *(.+?) *message *\*\/", file_content)
                if result:
                    head_list.append(file_name)
            except:
                print(f"2 find_msg_file_list file_name:{file_name}")
                return head_list
    return head_list
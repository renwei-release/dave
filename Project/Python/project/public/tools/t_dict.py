# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import json


# =====================================================================


def t_dict_save(file_path, dict):
    if isinstance(dict, str):
        dict = eval(dict)
    with open(file_path, 'w', encoding='utf-8') as f:
        str_ = json.dumps(dict, ensure_ascii=False, indent=4)
        f.write(str_)


def t_dict_load(file_path):
    dict = {}
    with open(file_path, 'r', encoding='utf-8') as f:
        data = f.read()
        dict = json.loads(data)
    return dict
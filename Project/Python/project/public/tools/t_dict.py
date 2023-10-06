# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import json
import random
from .t_file import *


# =====================================================================


def t_dict_save(file_path, dict):
    if isinstance(dict, str):
        dict = eval(dict)
    with open(file_path, 'w', encoding='utf-8') as f:
        str_ = json.dumps(dict, ensure_ascii=False, indent=4)
        f.write(str_)


def t_dict_load(file_path):
    dict = {}

    if t_path_or_file_exists(file_path) == False:
        print(f'load file:{file_path} not exists!')
        return dict

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            data = f.read()
            dict = json.loads(data)
    except Exception as e:
        print(f'load file:{file_path} error:{e}')
        dict = {}
    return dict


def t_dict_random(dicts):
    dict_key_ls = list(dicts.keys())
    random.shuffle(dict_key_ls)
    new_dict = {}
    for key in dict_key_ls:
        new_dict[key] = dicts.get(key)
    return new_dict
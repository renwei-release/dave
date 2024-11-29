# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import json
import random
from .t_file import *


# =====================================================================


def t_dict_save(file_path, dict_data):
    if isinstance(dict_data, str):
        dict_data = eval(dict_data)
    with open(file_path, 'w', encoding='utf-8') as f:
        str_ = json.dumps(dict_data, ensure_ascii=False, indent=4)
        f.write(str_)


def t_dict_load(file_path):
    dict_data = {}

    if t_path_or_file_exists(file_path) == False:
        print(f'load file:{file_path} not exists!')
        return dict_data

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            data = f.read()
            if len(data) > 0:
                dict_data = json.loads(data)
    except Exception as e:
        print(f'load file:{file_path} error:{e}')
        dict_data = {}

    return dict_data


def t_dict_merge(dict1, dict2):
    for key, value in dict2.items():
        if key in dict1:
            dict1[key] = dict1[key] + value
        else:
            dict1[key] = value
    return dict1


def t_dict_random(dict_data):
    dict_key_ls = list(dict_data.keys())
    random.shuffle(dict_key_ls)
    new_dict = {}
    for key in dict_key_ls:
        new_dict[key] = dict_data.get(key)
    return new_dict


def t_dict_hash(dict_data):
    return hash(str(dict_data))


def t_dict_hash_str(dict_data):
    return str(t_dict_hash(dict_data))


def t_dict_surround(dict_data):
    dict_key_ls = list(dict_data.keys())
    dict_value_ls = list(dict_data.values())

    head_data = {}

    new_dict = {}
    for i in range(len(dict_key_ls)):
        if i == 0:
            head_data = {dict_key_ls[0]: dict_value_ls[0]}
        elif i == len(dict_key_ls) - 1:
            new_dict.update(head_data)
        else:
            new_dict[dict_key_ls[i]] = dict_value_ls[i]

    return new_dict
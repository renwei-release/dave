# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import os
import string
import re
from .t_time import *
from .t_sys import *


def _t_path_file_list(file_path, suffix, recursive):
    file_number = 0
    file_list = []

    if os.path.exists(file_path) == False:
        return file_list, file_number

    for full_name in os.listdir(file_path):
        full_path = os.path.join(file_path, full_name)
        if os.path.isfile(full_path):
            if full_name.rsplit('.', 1)[-1] == suffix:
                file_number += 1
                file_list.append(full_path)
            else:
                if re.findall('.', full_name):
                    if suffix == None:
                        file_number += 1
                        file_list.append(full_path)
        else:
            if recursive == True:
                add_file_list, add_file_number = _t_path_file_list(file_path=full_path, suffix=suffix, recursive=recursive)
                file_list.extend(add_file_list)
                file_number += add_file_number
    return file_list, file_number


def _t_path_file_array_list_num(file_path_array, suffix, recursive):
    file_list = []
    total_num = 0
    for file_path in file_path_array:
        file_path_list, total_path_num = _t_path_file_list(file_path, suffix, recursive)
        file_list.extend(file_path_list)
        total_num += total_path_num
    return file_list, total_num


def _t_path_file_list_num(file_path, suffix, recursive):
    file_list = None
    total_num = None

    if type(file_path) == str:
        file_list, total_num = _t_path_file_list(file_path, suffix, recursive)
    elif type(file_path) == list:
        file_list, total_num = _t_path_file_array_list_num(file_path, suffix, recursive)

    return file_list, total_num


# =====================================================================


def t_path_or_file_exists(path_or_file):
    return os.path.exists(path_or_file)


def t_write_pic(pic_data, pic_name):
    with open(pic_name, 'wb') as f:
        f.write(pic_data)
    return


def t_creat_path(path:str):
    last_path = path.rsplit('/', 1)[-1]
    if last_path.find('.') >= 0:
        path = path.rsplit('/', 1)[0]
    if not t_path_or_file_exists(path):
        os.system(f"mkdir -p {path}")
    return


def t_path_child_list(father_path):
    if os.path.exists(father_path) == False:
        return []

    child_list = [os.path.join(father_path, name) for name in os.listdir(father_path)
        if os.path.isdir(os.path.join(father_path, name))]
    return child_list


def t_path_child_number(father_path):
    child_num = sum([os.path.isdir(os.path.join(father_path, listx)) for listx in os.listdir(father_path)])
    return child_num


def t_path_file_list(file_path=None, suffix='jpg', recursive=True):
    return _t_path_file_list_num(file_path, suffix, recursive)


def t_path_file_number(path):
    file_number = 0
    for item in os.listdir(path):
        if os.path.isfile(os.path.join(path, item)):
            file_number += 1
        else:
            file_number += t_path_file_number(os.path.join(path, item))
    return file_number


def t_file_replace_line(file_path, line_flag, replace_data):
    if not replace_data.endswith('\n'):
        replace_data = replace_data + '\n'

    read_file = open(file_path, 'r')
    write_data = []
    for read_line in read_file.readlines():
        if read_line.find(line_flag) < 0:
            write_data.append(read_line)
        else:
            write_data.append(replace_data)
    read_file.close()

    write_file = open(file_path, 'w')
    write_file.writelines(write_data)
    write_file.close()
    return


def t_file_line_to_array(file_name):
    contentall = []
    with open(file_name, 'r', encoding='utf-8') as file:
        content_list = file.readlines()
        contentall = [x.strip() for x in content_list]
    return contentall
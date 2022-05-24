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


def _t_train_path_last_dir_name(train_path):
    if type(train_path) == str:
        last_dir_name = train_path.rsplit('/', 1)[-1]
    elif type(train_path) == list:
        last_dir_name = []
        for train_path_sub in train_path:
            last_dir_name.append(train_path_sub.rsplit('/', 1)[-1])
    return last_dir_name


def _t_train_path_second_last_dir_name(train_path):
    if type(train_path) == str:
        second_dir_name = train_path.rsplit('/', 1)[0].rsplit('/', 1)[-1]
    elif type(train_path) == list:
        second_dir_name = []
        for train_path_sub in train_path:
            second_dir_name.append(train_path_sub.rsplit('/', 1)[0].rsplit('/', 1)[-1])
    return second_dir_name


def _t_train_output_data_1(output_data):
    output_str = None
    if type(output_data) == str:
        output_str = output_data
    elif type(output_data) == list:
        for output_data_sub in output_data:
            if output_str == None:
                output_str = output_data_sub
            else:
                output_str = output_str + '_' + output_data_sub
    return output_str


def _t_train_output_data_2(output_data_1, output_data_2):
    output_str = None
    if type(output_data_1) == str:
        output_str = output_data_1 + '_' + output_data_2
    elif type(output_data_1) == list:
        for output_data_sub_1, output_data_sub_2 in zip(output_data_1, output_data_2):
            output_data_sub_12 = output_data_sub_1 + '-' + output_data_sub_2
            if output_str == None:
                output_str = output_data_sub_12
            else:
                output_str = output_str + '_' + output_data_sub_12
    return output_str


def _t_train_last_dir_name_has_special_character(last_dir_name):
    last_dir_name = last_dir_name.rsplit('/', 1)[-1]
    last_dir_name = _t_train_output_data_1(last_dir_name)
    if last_dir_name == 'train' \
        or last_dir_name == 'test' \
        or last_dir_name == 'total' \
        or last_dir_name == 'val' \
        or last_dir_name == 'tfrecord' >= 0:
        return True
    else:
        return False


def _t_train_path_to_model_path_get_train_name(train_path, train_name):
    last_dir_name = _t_train_path_last_dir_name(train_path)
    if train_name == None:
        last_second_dir_name = _t_train_path_second_last_dir_name(train_path)
        if _t_train_last_dir_name_has_special_character(last_dir_name):
            train_name = _t_train_output_data_2(last_second_dir_name, last_dir_name)
        else:
            train_name = _t_train_output_data_1(last_dir_name)
    return train_name


def _t_path_file_list(file_path, suffix, recursive):
    file_number = 0
    file_list = []
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


def _t_picture_file_array_list_num(file_path_array, suffix, recursive):
    file_list = []
    total_num = 0
    for file_path in file_path_array:
        file_path_list, total_path_num = _t_path_file_list(file_path, suffix, recursive)
        file_list.extend(file_path_list)
        total_num += total_path_num
    return file_list, total_num


def _t_picture_file_list_num(file_path, suffix, recursive):
    file_list = None
    total_num = None

    if type(file_path) == str:
        file_list, total_num = _t_path_file_list(file_path, suffix, recursive)
    elif type(file_path) == list:
        file_list, total_num = _t_picture_file_array_list_num(file_path, suffix, recursive)

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
    child_list = [os.path.join(father_path, name) for name in os.listdir(father_path)
        if os.path.isdir(os.path.join(father_path, name))]
    return child_list


def t_path_child_number(father_path):
    child_num = sum([os.path.isdir(os.path.join(father_path, listx)) for listx in os.listdir(father_path)])
    return child_num


def t_path_file_list(file_path=None, suffix='jpg', recursive=True):
    return _t_picture_file_list_num(file_path, suffix, recursive)


def t_path_file_number(path):
    file_number = 0
    for item in os.listdir(path):
        if os.path.isfile(os.path.join(path, item)):
            file_number += 1
        else:
            file_number += t_path_file_number(os.path.join(path, item))
    return file_number


def t_train_path_to_model_path(train_path, model_name, model_suffix=None, train_name=None, misc_str=None):
    train_name = _t_train_path_to_model_path_get_train_name(train_path, train_name)
    model_user_path = t_sys_model_user_path()
    if misc_str == None:
        pre_path_str = model_user_path+'/'+model_name+'_'+train_name
    else:
        pre_path_str = model_user_path+'/'+model_name+'_'+train_name+'_'+misc_str
    if model_suffix != None:
        model_file = pre_path_str+'_'+t_time_current_str()+'.'+model_suffix
    else:
        model_file = pre_path_str+'_'+t_time_current_str()
    model_file = model_file.replace('.', '_')
    t_creat_path(model_file)
    print(f'the model path:{model_file}')
    return model_file


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
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
from autocode_tools import *


def _is_invalid_name_char(end_char):
    if (end_char == '#') \
        or (end_char == '(') \
        or (end_char == ')') :
        return True
    else:
        return False


def _is_define_name_end(end_char):
    if (((end_char >= 'a') and (end_char <= 'z')) \
        or ((end_char >= 'A') and (end_char <= 'Z')) \
        or ((end_char >= '0') and (end_char <= '9')) \
        or (end_char == '_')):
        return False
    else:
        return True


def _is_invalid_value_char(end_char):
    if end_char == '#':
        return True
    else:
        return False


def _is_define_value_joiner(end_char):
    if (end_char == '+') \
        or (end_char == '-') \
        or (end_char == '*') \
        or (end_char == '/') \
        or (end_char == ' '):
        return True
    else:
        return False


def _is_invalid_value(define_value):
    if (define_value == 'void') \
        or (define_value == 'typedef'):
        return True
    else:
        return False


def _load_define_list(define_list, content_ptr):
    content_ptr = ' '.join(content_ptr.split())
    content_len = len(content_ptr)

    load_buffer = ''

    find_define_head = False
    find_define_name = False
    find_define_value = False

    define_name = ''
    define_value = ''

    for content_index in range(content_len):
        load_buffer += content_ptr[content_index]

        if find_define_head == False:
            if len(load_buffer) <= 8:
                if load_buffer == '#define '[0:len(load_buffer)]:
                    if len(load_buffer) == 8:
                        find_define_head = True
                        load_buffer = ''
                else:
                    load_buffer = ''
        elif find_define_name == False:
            if _is_invalid_name_char(content_ptr[content_index]) == True:
                load_buffer = content_ptr[content_index]
                find_define_head = False
                find_define_name = False
                find_define_value = False
            elif _is_define_name_end(content_ptr[content_index]) == True:
                define_name = load_buffer[0:len(load_buffer)-1]
                load_buffer = ''
                find_define_name = True
        elif find_define_value == False:
            if _is_invalid_value_char(content_ptr[content_index]) == True:
                load_buffer = content_ptr[content_index]
                find_define_head = False
                find_define_name = False
                find_define_value = False
            elif content_ptr[content_index] == ' ' or content_ptr[content_index] == ')':
                if (content_index + 1) < content_len:
                    if _is_define_value_joiner(content_ptr[content_index + 1]) == False:
                        if _is_define_value_joiner(content_ptr[content_index - 1]) == False:
                            find_define_value = True
                else:
                    find_define_value = True

        if find_define_value == True:
            find_define_head = False
            find_define_name = False
            find_define_value = False
            define_value = load_buffer[0:-1]
            load_buffer = ''
            define_name = define_name.replace(' ', '')
            define_value = define_value.replace(' ', '').replace('(', '').replace(')', '')
            if _is_invalid_value(define_value) == False:
                define_list.append(f'#define {define_name} {define_value}')
    return


def _find_define_list(file_list=None):
    define_list = []
    head_list = []
    for file_name in file_list:
        with open(file_name, encoding="utf-8") as file_id:
            try:
                file_content = file_id.read()
            except:
                print(f"1 _find_define_list file_name:{file_name}")
                return define_list, head_list
            try:
                file_content = remove_annotation_data(file_content)

                if _load_define_list(define_list, file_content) == True:
                    head_list.append(file_name)
            except:
                print(f"2 _find_define_list file_name:{file_name}")
                return define_list, head_list
    return define_list, head_list


def _find_define_digital_table(define_table, define_list):
    for define_data in define_list:
        define_name_list = re.findall('#define +([a-z,A-Z,0-9,_]+) +[0-9,*]+', define_data)
        define_value_list = re.findall('#define +[a-z,A-Z,0-9,_]+ +([0-9,*]+)', define_data)
        define_name = get_array_data(define_name_list, 0)
        define_value = get_array_data(define_value_list, 0)
        if (define_name != None) and (define_value != None):
            define_table[define_name] = define_value
    return


def _find_define_hex_table(define_table, define_list):
    for define_data in define_list:
        define_name_list = re.findall('#define +([a-z,A-Z,0-9,_]+) 0x[0-9,a-f,A-F]+', define_data)
        define_value_list = re.findall('#define +[a-z,A-Z,0-9,_]+ (0x[0-9,a-f,A-F]+)', define_data)
        define_name = get_array_data(define_name_list, 0)
        define_value = get_array_data(define_value_list, 0)
        if (define_name != None) and (define_value != None):
            define_table[define_name] = define_value
    return


def _find_define_complex_table(define_table, define_list):
    for define_data in define_list:
        define_name_list = re.findall("#define +([a-z,A-Z,0-9,_]+) +[0-9,a-z,A-Z,+,-,*,/, ,_]+", define_data)
        define_value_list = re.findall("#define +[a-z,A-Z,0-9,_]+ +([0-9,a-z,A-Z,+,-,*,/, ,_]+)", define_data)
        define_name = get_array_data(define_name_list, 0)
        define_value = get_array_data(define_value_list, 0)
        if (define_name != None) and (define_value != None):
            define_table[define_name] = define_value
    return


def _the_define_value_has_define_name(define_value, define_table, new_define_table):
    sub_define_value = ''
    for value_index in range(len(define_value)):
        if (define_value[value_index] == '*') \
            or (define_value[value_index] == '+') \
            or (define_value[value_index] == '-') \
            or (define_value[value_index] == '/') \
            or ((value_index + 1) >= len(define_value)):

            if (value_index + 1) >= len(define_value):
                sub_define_value += define_value[value_index]

            if define_table.get(sub_define_value, None) != None:
                new_define_table[sub_define_value] = define_table.get(sub_define_value, None)
            sub_define_value = ''
        else:
            sub_define_value += define_value[value_index]
    return


def _remove_struct_unuse_define(define_table, all_struct_table):
    base_define_table = {}
    for define_name in define_table.keys():
        if define_on_the_table(define_name, all_struct_table) == True:
            base_define_table[define_name] = define_table[define_name]

    new_define_table = {}
    for Loop_multiple_times_to_get_nested_define in range(3):
        loop_define_table = {}
        for base_define_name in base_define_table.keys():
            base_define_value = base_define_table[base_define_name]
            if base_define_value != '':
                _the_define_value_has_define_name(base_define_value, define_table, loop_define_table)
        new_define_table.update(loop_define_table)

    new_define_table.update(base_define_table)
    return new_define_table


# =====================================================================


def find_define_table(file_list, all_struct_table):
    define_table = {}
    define_list, include_list = _find_define_list(file_list)

    _find_define_digital_table(define_table, define_list)
    _find_define_hex_table(define_table, define_list)
    _find_define_complex_table(define_table, define_list)

    define_table = _remove_struct_unuse_define(define_table, all_struct_table)

    return define_table, include_list
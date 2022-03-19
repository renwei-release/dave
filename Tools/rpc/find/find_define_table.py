# -*- coding: utf-8 -*-

#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.03.11.
#

import re
from rpc_tools import *


def _find_define_valid_string(string_data):
    if re.search('[A-Z,a-z,0-9,"_"]+?', string_data):
        return True
    return False


def __find_define_list(define_list, define_data):
    find_flag = 0
    for define_string in define_data:
        define_string = define_string.rsplit('//')[0].strip()
        define_string = define_string.replace('\t', ' ')
        end_string = define_string.split(' ', 2)[-1].strip()
        end_string = end_string.replace('(', '').replace(')', '').replace(' ', '').replace('*', '').replace('+', '')
        if _find_define_valid_string(end_string):
            find_flag = 1
            define_string = define_string.replace("(", "").replace(")", "").replace("\r", "").replace("\n", "")
            define_list.append(define_string)
        else:
            print(f"end_string:{end_string} define_string:{define_string}")
    return find_flag


def _find_define_list():
    define_list = []
    head_list = []
    file_list = find_file_list()
    for file_name in file_list:
        with open(file_name, encoding="utf-8") as file_id:
            try:
                file_content = file_id.read()
            except:
                print(f"file_name:{file_name}")
                traceback.print_exc()
                return define_list, head_list
            try:
                result = re.findall('#define [A-Z,a-z,0-9,"_"]+? ["(",")",A-Z,a-z,0-9,"_"]+?\n', file_content)
                if result:
                    if _find_define_list(define_list, result) == 1:
                        head_list.append(file_name)
            except:
                print(f"file_name:{file_name}")
                traceback.print_exc()
                return define_list, head_list
    return define_list, head_list


def _find_define_digital_table(define_dictionary, define_list=None):
    for define_data in define_list:
        define_name_list = re.findall("#define (.*?) .*?", define_data)
        define_value_list = re.findall("#define .*? ([0-9, *, +]+|0[x,X][a-f, A-F]+)", define_data)
        define_name = get_array_data(define_name_list, 0)
        define_value = get_array_data(define_value_list, 0)
        if define_name != None and define_value != None:
            define_name.replace(" ", "")
            define_value.replace(" ", "")
            define_dictionary[define_name] = eval(define_value)
    return


def _find_define_complex_table(define_dictionary, define_list=None):
    for define_data in define_list:
        define_name_list = re.findall("#define (.*?) .*?", define_data)
        define_value_list = re.findall("#define .*? ([a-z, A-Z, _]+[0-9, a-z, A-Z, _]+)", define_data)
        define_name = get_array_data(define_name_list, 0)
        define_value = get_array_data(define_value_list, 0)
        if define_name != None and define_value != None:
            define_name.replace(" ", "")
            define_value.replace(" ", "")
            define_dictionary[define_name] = define_dictionary.get(define_value, 0)
    return


# =====================================================================


def find_define_table():
    define_table = {}
    define_list, head_list = _find_define_list()
    _find_define_digital_table(define_table, define_list=define_list)
    _find_define_complex_table(define_table, define_list=define_list)
    return define_table
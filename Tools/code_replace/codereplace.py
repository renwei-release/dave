#!/usr/bin/python3
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import sys
import os
import re


def _codereplace_main(file_list, src_code, dst_code):
    for file_name in file_list:
        with open(file_name, "r", encoding="utf-8") as file_id:
            try:
                file_data = file_id.read()
                modify_data = re.sub(src_code, dst_code, file_data)
            except:
                file_data = ''
                modify_data = ''
                print(f'read file:{file_name} error!')
        if modify_data != file_data:
            print(f'file:{file_name} modify {src_code}->{dst_code}!')
            with open(file_name, "w", encoding="utf-8") as file_id:
                file_id.write(modify_data)
    return


def __codereplace_file_list(file_list, file_path):
    current_file_or_dir_list = os.listdir(file_path)
    for current_file_or_dir in current_file_or_dir_list:
        current_file_or_dir = file_path + '/' + current_file_or_dir
        if os.path.isdir(current_file_or_dir):
            __codereplace_file_list(file_list, current_file_or_dir)
        else:
            file_list.append(current_file_or_dir)
    return


def _codereplace_file_list(file_path):
    file_list = []
    if file_path.endswith('/'):
        file_path = file_path[:-1]
    __codereplace_file_list(file_list, file_path)
    return file_list


# =====================================================================

#
# python codereplace.py ../../C errorstr retstr
#
def codereplace_main():
    if len(sys.argv) < 3:
        print(f'give invalid param:{sys.argv}')
        return

    file_path = sys.argv[1]
    src_code = sys.argv[2]
    dst_code = sys.argv[3]

    print(f'file_path:{file_path} {src_code}->{dst_code}')

    file_list = _codereplace_file_list(file_path)

    _codereplace_main(file_list, src_code, dst_code)


if __name__ == '__main__':
    codereplace_main()
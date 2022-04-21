#!/usr/bin/python3
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import os


def _cleanedup_del_dir(path_full, path_name):
    if path_name == '__pycache__':
        os.system(f'rm -rf {path_full}')
        return True

    return False


def _cleanedup_del_file(file_path, file_name):
    if '-BIN' in file_name:
        os.system(f'rm -rf {file_path}')
        return True

    return False


def _cleanedup_search_path(the_path):
    the_path_files = os.listdir(the_path)
    for the_path_file in the_path_files:
        full_path_file = os.path.join(the_path, the_path_file)
        if os.path.isdir(full_path_file):
            if _cleanedup_del_dir(full_path_file, the_path_file) == False:
                _cleanedup_search_path(full_path_file)
            else:
                print(f'Cleaned up dir:{full_path_file}')
        elif os.path.isfile(full_path_file):
            if _cleanedup_del_file(full_path_file, the_path_file) == True:
                print(f'Cleaned up file:{full_path_file}')
    return


# =====================================================================


#
# python cleanedup.py
#
def cleanedup_main():

    root_path = '../../'
    _cleanedup_search_path(root_path)
    return


if __name__ == '__main__':
    cleanedup_main()
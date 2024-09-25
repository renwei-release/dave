# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import os
import sys
import subprocess
import getpass


def t_sys_dataset_path():
    return '/project/dataset'


def t_sys_model_path():
    return '/project/model'


def t_sys_user_name():
    username = getpass.getuser()
    return username


def t_sys_pc_name():
    hostname = os.popen('hostname').read().replace('\r', '').replace('\n', '')
    return hostname


def t_sys_model_user_path():
    return '/project/model/trained_model/' + t_sys_pc_name()


def t_sys_path_append(path=None):
    current_file = sys._getframe(1).f_code.co_filename
    current_file_absolute_path = os.path.dirname(os.path.abspath(current_file))
    if path != None:
        sys.path.append(current_file_absolute_path+path)
    else:
        sys.path.append(current_file_absolute_path)
    return


def t_sys_path_show():
    print(f"python package search path:{sys.path}")
    return


def t_sys_path_file_number(path):
    file_number = 0
    file_list = os.listdir(path)
    for file_name in file_list:
        if os.path.isdir(os.path.join(path, file_name)):
            file_number += t_sys_path_file_number(os.path.join(path, file_name))
        else:
            file_number += 1
    return file_number


def t_sys_myline(depth):
    __func__ = sys._getframe(depth).f_code.co_name.encode("utf-8")
    __LINE__ = sys._getframe(depth).f_lineno
    return __func__, __LINE__


def t_sys_gpu_name():
    try:
        result = subprocess.run(['nvidia-smi', '--query-gpu=name', '--format=csv,noheader'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        if result.returncode != 0:
            raise RuntimeError(f"nvidia-smi command failed with error: {result.stderr}")

        gpu_models = result.stdout.strip().split('\n')
        return gpu_models[0]
    except FileNotFoundError:
        raise RuntimeError("nvidia-smi command not found. Ensure that NVIDIA drivers are installed and nvidia-smi is in your PATH.")


def t_sys_pyInstaller_temp_dir():
    if hasattr(sys, '_MEIPASS'):
        # PyInstaller creates a temp folder and stores path in _MEIPASS
        temp_dir = sys._MEIPASS
    elif len(sys.argv) >= 3:
        # Fallback to the current directory if not running as a PyInstaller bundle
        # command = [main_program, auto_argument, sys._MEIPASS] + sys.argv[1:]
        temp_dir = sys.argv[2]
    else:
        # Fallback to the current directory if not running as a PyInstaller bundle
        temp_dir = '/project'

    if not os.path.isdir(temp_dir):
        return '/project'

    return temp_dir


def t_sys_attributes():
    attributes = dir(sys)
    return attributes
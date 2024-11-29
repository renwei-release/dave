# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import subprocess
import sys
import os


def _env_check():
    try:
        import cryptocode
    except ImportError:
        os.system('pip3 install cryptocode')


# =====================================================================


_env_check()


auto_argument = "___FLAG_FOR_PRODUCT___"
print(f"auto_argument:{auto_argument}")

current_dir = os.path.dirname(os.path.abspath(__file__))
print(f"current_dir:{current_dir}")
main_program = os.path.join(current_dir, 'dave_main.py')
print(f"main_program:{main_program}")

python_path = os.pathsep.join(sys.path)
os.environ['PYTHONPATH'] = python_path

command = ['python3', main_program, auto_argument, sys._MEIPASS] + sys.argv[1:]
print(f"command:{command}")

subprocess.run(command)
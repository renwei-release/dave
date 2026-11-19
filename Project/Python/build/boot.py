# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2026 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import subprocess
import sys
import os
import shutil


def _program_info(program_name):
    program_path = shutil.which(program_name)
    program_version = 'NOT_FOUND'
    if program_path:
        try:
            version_result = subprocess.run(
                [program_name, '--version'],
                check=True,
                capture_output=True,
                text=True,
            )
            program_version = (version_result.stdout or version_result.stderr).strip()
        except Exception as exc:
            program_version = f'UNKNOWN ({exc})'
    return program_path, program_version


def _env_print():
    print('\n=================')
    print(f"boot BIN name: {sys.executable}")
    print(f"boot BIN python version: {sys.version}")
    print(f"boot BIN python path: {sys.path}")

    python3_path, python3_version = _program_info('python3')
    gcc_path, gcc_version = _program_info('gcc')

    print(f"local system PYTHONPATH: {os.environ.get('PYTHONPATH', '')}")
    print(f"local system python3: {python3_path}/{python3_version}")
    print(f"local system gcc: {gcc_path}/{gcc_version}")
    print('==================\n')
    return


def _env_check():
    try:
        import cryptocode
    except ImportError:
        os.system('pip3 install cryptocode --break-system-packages')


# =====================================================================


_env_print()
_env_check()


auto_argument = "___FLAG_FOR_PRODUCT___"
print(f"auto_argument:{auto_argument}")

current_dir = os.path.dirname(os.path.abspath(__file__))
print(f"current_dir:{current_dir}")
main_program = os.path.join(current_dir, 'dave_main.py')
print(f"main_program:{main_program}")

command = ['python3', main_program, auto_argument, sys._MEIPASS] + sys.argv[1:]
print(f"command:{command}")

subprocess.run(command)
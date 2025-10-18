# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2025 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import ast
import os
import sys
import shutil


if __name__ == '__main__':
    project_name = sys.argv[1]

    src_project_home = '../../../../project'
    dst_project_home = f'../../../../../../Deploy/deploy/{project_name}/file_system/project'

    src_weights_home = f"{src_project_home}/weights/{project_name}"
    dst_weights_home = f"{dst_project_home}/weights/{project_name}"

    if os.path.exists(src_weights_home):
        if not os.path.exists(dst_weights_home):
            os.makedirs(dst_weights_home)

        shutil.copytree(src_weights_home, dst_weights_home, dirs_exist_ok=True)
        os.system(f'chmod -R 777 {dst_weights_home}')
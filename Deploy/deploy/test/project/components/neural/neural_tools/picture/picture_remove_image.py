#!/usr/bin/python
#
# ================================================================================
# (c) Copyright 2022 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2022.02.01.
# ================================================================================
#
import os
from public.tools import *


def _t_picture_remove_image(file_list, remove_array):
    for file_name in file_list:
        file_id = file_name.rsplit('/', 1)[-1].rsplit('.', 1)[0]
        if file_id in remove_array:
            print(f'remove file:{file_name}')
            os.system(f'rm -rf {file_name}')


# =====================================================================

#
# 从remove_path里面移除remove_file文件列表中的文件。
#
def t_picture_remove_image(file_path, remove_file):
    file_list, total_num = t_path_file_list(file_path)
    remove_array = t_file_line_to_array(remove_file)
    _t_picture_remove_image(file_list, remove_array)


if __name__ == "__main__":
    file_path = '/project/dataset/Private/Painting'
    remove_file = './remove_file_list.txt'
    t_picture_remove_image(file_path, remove_file)
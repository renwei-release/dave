#!/usr/bin/python3
# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.06.26.
# ================================================================================
#
import io
import sys
import traceback
from PIL import Image
from tensorflow import keras
from public.tools import *


def _keras_check_the_picture_can_be_read(picture_file):
    image_data = keras.preprocessing.image.load_img(picture_file)
    if image_data is None:
        print(f"{picture_file} on keras can't open!")
        ret = False
    else:
        ret = True
    return ret


def _pil_check_the_picture_can_be_read(picture_file):
    image_data = Image.open(picture_file)
    if image_data is None:
        print(f"{picture_file} on pil can't open!")
        ret = False
    else:
        #
        # 有些过大的图片，通过Pillow做resize会出错
        # 可能和Pillow的ImageFile.py文件的以下两个标注值有关：
        # MAXBLOCK = 65536
        # SAFEBLOCK = 1024 * 1024
        # 或者损毁的图片，也能通过resize被发现
        #
        image_data = image_data.resize((320, 320), Image.BILINEAR)
        if image_data.mode != 'RGB':
            print(f"conversion {picture_file}({image_data.mode}) to RBG")
            image_data = image_data.convert("RGB")
            image_data.save(picture_file)
            ret = True
        else:
            ret = True
    return ret


def _check_the_picture_can_be_read(picture_file):
    ret = False
    try:
        if _keras_check_the_picture_can_be_read(picture_file) == True:
            if _pil_check_the_picture_can_be_read(picture_file) == True:
                ret = True
    except:
        traceback.print_exc()
    return ret


def _jpg_format_conversion(picture_path):
    invalid_file = []
    file_list, file_number = t_path_file_list(picture_path)
    progress_bar = t_print_progress_bar(file_number)

    for picture_file in file_list:
        if _check_the_picture_can_be_read(picture_file) == False:
            picture_file = bytes(picture_file, encoding='utf-8', errors='xmlcharrefreplace').decode('utf-8')
            print(f"the picture_file:{picture_file} can't read!")
            invalid_file.append(picture_file)
        progress_bar.show()
    return invalid_file


# =====================================================================

#
# 检查图片是否可被正确读入，不能读入的图片打印图片名称，
# 如果能做转换，会被自动转换
#
def t_picture_jpg_format_conversion(path):
    _jpg_format_conversion(path)
    return


if __name__ == "__main__":
    t_picture_jpg_format_conversion(*sys.argv[1:])
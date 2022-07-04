# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.07.07.
# ================================================================================
#
import re
import base64
from io import BytesIO
from PIL import Image


def _pil_base64_open(base64_data):
    base64_data = re.sub('^data:image/.+;base64,', '', base64_data)
    byte_data = base64.b64decode(base64_data)
    image_data = BytesIO(byte_data)
    image_pic = Image.open(image_data)
    return image_pic


def _pil_file_open(file_path):
    return Image.open(file_path)


# =====================================================================


def pil_open(image_info):
    image_pic = None

    if type(image_info).__name__ == 'bytes':
        # 二进制数据
        image_pic = Image.open(BytesIO(image_info))
    elif type(image_info).__name__ == 'ndarray':
        # 数组数据
        image_pic = image_info
    elif type(image_info).__name__ == 'str':
        if len(image_info) > 1024:
            # 如果长度大于1024字节，默认是base64数据
            image_pic = _pil_base64_open(image_info)
        else:
            # 如果长度小于等于1024字节，默认是传递的图片路径
            image_pic = _pil_file_open(image_info)
    else:
        print(f'pil_open can\'t open {type(image_info).__name__}')
        return None

    if (image_pic != None) and (image_pic.mode != 'RGB'):
       image_pic = image_pic.convert('RGB')

    return image_pic
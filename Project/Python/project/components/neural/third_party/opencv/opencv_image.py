# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.07.14.
# ================================================================================
#
import cv2
import base64
import re
from io import BytesIO
import numpy as np

base64_code = "^([A-Za-z0-9+/]{4})*([A-Za-z0-9+/]{4}|[A-Za-z0-9+/]{3}=|[A-Za-z0-9+/]{2}==)$"

def _opencv_file_open(file_path):
    return cv2.imread(file_path)


def _opencv_bytes_open(bytes_data):
    return cv2.imdecode(np.frombuffer(bytes_data, np.uint8), cv2.IMREAD_COLOR)


def _opencv_base64_open(base64_data):
    base64_data = re.sub('^data:image/.+;base64,', '', base64_data)
    byte_data = base64.b64decode(base64_data)
    image_data = _opencv_bytes_open(byte_data)
    return image_data


# =====================================================================


def opencv_open(image_info=None, BGRFLAG=0):
    image_pic = None

    if isinstance(image_info, bytes):
        # 二进制数据
        image_pic = _opencv_bytes_open(image_info)
    elif isinstance(image_info, np.ndarray):
        # 数组数据
        image_pic = image_info
    elif isinstance(image_info, str):
        # 判断字符串是否为base64格式数据
        if bool(re.match(base64_code, image_info)):
            
            image_pic = _opencv_base64_open(image_info)
        else:
            
            image_pic = _opencv_file_open(image_info)
    else:
        print(f'opencv_open can\'t open {type(image_info).__name__}')
        return None

    if image_pic is not None:
        if BGRFLAG == 0:
            image_pic = cv2.cvtColor(image_pic, cv2.COLOR_BGR2RGB)
    else:
        print(f'opencv can\'t open:{image_info}')

    return image_pic


def opencv_save(image_info=None, image_name=None):
    if image_name == None:
        cv2.imwrite('opencv_image_info.jpg', image_info)
    else:
        cv2.imwrite(image_name, image_info)
    return

# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.06.28.
# yolov5-5.0的代码来自一下链接：
# https://github.com/ultralytics/yolov5/tags
#
# 运行容器环境cuda11
#
# ================================================================================
#
from public.tools import *
from components.neural.model.YOLO.model_predict import predict as yolo_predict


# =====================================================================


def test(
        test_path='/project/dataset/Private/renwei/yolo',
    ):
    yolo = yolo_predict()

    file_list, file_number = t_path_file_list(test_path)
    for file_name in file_list:
        feature_array = yolo.feature(image_info=file_name, user_want_class=0)
        print(f"file:{file_name.rsplit('/', 1)[-1]} feature_array:{feature_array}")

    return


if __name__ == "__main__":
    test()
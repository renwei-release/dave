# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.07.14.
#
# 运行容器环境 cuda10
#
# ================================================================================
#
import time
from model_predict import predict as keynet_predict
from public.tools import *

# =====================================================================


def test(
        test_path = '/project/dataset/Private/Painting',
    ):
    keynet = keynet_predict()

    file_list, file_number = t_path_file_list(test_path)
    file_counter = 0
    start_time = time.time()
    for file_name in file_list:
        pts, desc = keynet.predict(file_name, 500, 500)
        print(f"file:{file_name.rsplit('/', 1)[-1]} pts:{pts} desc:{desc}")
        file_counter += 1
    end_time = time.time()

    print(f"总数量：{file_counter} 总耗时：{end_time - start_time} 均速度：{(end_time - start_time)/file_counter}")
    return


if __name__ == "__main__":
    test()
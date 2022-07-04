# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.07.13.
# Key.Net用来提取图像特征，类似SIFT：
# https://github.com/cavalli1234/AdaLAM/archive/refs/heads/master.zip
#
# 运行容器环境cuda10
#
# ================================================================================
#
import time
from public.tools import *
from model_predict import predict as adalam_predict


# =====================================================================


def test(
        test_path = '/project/dataset/Private/renwei',
    ):
    adalam = adalam_predict()

    file_list, file_number = t_path_file_list(test_path)
    file_counter = 0
    start_time = time.time()
    for file_name in file_list:
        match_number = adalam.picture_similarity(file_name, file_name)
        print(f"file:{file_name.rsplit('/', 1)[-1]} match_number:{match_number}")
        file_counter += 1
    end_time = time.time()

    print(f"总数量：{file_counter} 总耗时：{end_time - start_time} 均速度：{(end_time - start_time)/file_counter}")
    return


if __name__ == "__main__":
    test()
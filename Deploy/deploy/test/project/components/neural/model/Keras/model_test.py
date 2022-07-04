# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.06.21.
#
# 运行容器环境cuda11
#
# ================================================================================
#
import time
from Keras.model_predict import predict as keras_predict
from public.tools import *


def _predict(keras, test_file, right_label):
    pred_label = keras.predict(test_file)
    if pred_label == int(right_label):
        return True
    else:
        return False


def _feature(keras, test_file):
    return keras.feature(test_file)


def _path_label(test_path):
    path_list = t_path_child_list(test_path)
    path_label = []
    for child_path in path_list:
        path_label.append(child_path.split("/")[-1])
    return path_label


def _right_label(path_label, file_name):
    label_id = 0
    for label_name in path_label:
        if file_name.find(label_name) >= 0:
            return label_id
        label_id += 1
    return -1


def _test_predict(test_path):
    file_list, total_num = t_path_file_list(test_path)
    path_label = _path_label(test_path)
    progress_bar = t_print_progress_bar(total_num)
    keras = keras_predict()
    acc_num = 0

    start_time = time.time()
    for file_name in file_list:
        right_label = _right_label(path_label, file_name)
        if _predict(keras, file_name, right_label) == True:
            acc_num += 1
        progress_bar.show()
    end_time = time.time()

    print(f'总数量：{total_num}', end=' ')
    print(f'正确量：{acc_num}', end=' ')
    print(f'正确率：{acc_num/total_num}', end=' ')
    print(f'总耗时：{end_time-start_time}秒', end=' ')
    print(f'平均耗时：{(end_time-start_time)/total_num}秒')
    return file_name


def _test_feature(test_path):
    keras = keras_predict()

    file_list, file_num = t_path_file_list(test_path)
    for file_name in file_list:
        feature_data = _feature(keras, file_name)
        print(f"the test file {file_name} feature shape is {feature_data.shape}")
    return


# =====================================================================


def test(
        test_path='/project/dataset/Private/PhotographicAesthetics/test',
    ):
    _test_predict(test_path)
    _test_feature(test_path)
    return


if __name__ == "__main__":
    test()
# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.08.03.
# ================================================================================
#
from public.tools import *
from components.neural.model.Keras.model_predict import predict as keras_predict
from components.neural.model.DeepLab.model_predict import predict as deeplab_predict
from components.neural.model.MediaPipe.model_predict import predict as mediapipe_predict


def _t_picture_feature_array_keras(file_list, total_num):
    feature_dimension = 1024
    keras = keras_predict()
    progress_bar = t_print_progress_bar(total_num)
    feature_array = []
    feature_ids = {}
    feature_index = 0

    for file_name in file_list:
        feature = keras.feature(file_name)
        if len(feature) == feature_dimension:
            feature_array.append(feature)
            feature_ids.update({feature_index: file_name})
            feature_index += 1
        progress_bar.show()
    return total_num, feature_array, feature_ids, feature_dimension, keras.model_name()


def _t_picture_feature_array_deeplab(file_list, total_num):
    feature_dimension = 1024
    deeplab = deeplab_predict()
    progress_bar = t_print_progress_bar(total_num)
    feature_array = []
    feature_ids = {}
    feature_index = 0

    for file_name in file_list:
        feature = deeplab.feature(file_name, feature_dimension)
        if len(feature) == feature_dimension:
            feature_array.append(feature)
            feature_ids.update({feature_index: file_name})
            feature_index += 1
        progress_bar.show()
    return total_num, feature_array, feature_ids, feature_dimension, 'deeplab'


def _t_picture_feature_array_pose(file_list, total_num):
    feature_dimension = 1024
    mediapipe = mediapipe_predict()
    progress_bar = t_print_progress_bar(total_num)
    feature_array = []
    feature_ids = {}
    feature_index = 0

    for file_name in file_list:
        try :
            feature = mediapipe.feature(file_name, feature_dimension)
        except RuntimeError:
            print(f'file:{file_name} process error!')
            mediapipe = mediapipe_predict()
            feature = []
        if len(feature) == feature_dimension:
            feature_array.append(feature)
            feature_ids.update({feature_index: file_name})
            feature_index += 1
        progress_bar.show()
    return total_num, feature_array, feature_ids, feature_dimension, 'pose'


# =====================================================================


#
# 这个函数返回file_path路径下的图片特征集feature_array
# 同时还会返回图片存储ID与图片路径与名称对应的字典，可存储起来方便
# t_picture_feature_ids_to_file_ids函数使用。
#
def t_picture_feature_array(file_path, model=None):
    file_list, total_num = t_path_file_list(file_path)
    if file_list == None:
        return None, None, None, None, None

    print(f't_picture_feature_array file_path:{file_path} total_num:{total_num}')

    if model == 'vgg16':
        return _t_picture_feature_array_keras(file_list, total_num)
    elif model == 'deeplab':
        return _t_picture_feature_array_deeplab(file_list, total_num)
    elif model == 'pose':
        return _t_picture_feature_array_pose(file_list, total_num)
    else:
        return _t_picture_feature_array_keras(file_list, total_num)


#
# 模型中只能存储数字ID id_array，这个函数把数字ID与实际的文件名称对应 feature_ids。
#
def t_picture_feature_ids_to_file_ids(id_array, feature_ids):
    file_names_array = []
    file_ids_array = []
    for id in id_array:
        file_id = feature_ids.get(str(id), None)
        file_names_array.append(file_id)
        file_ids_array.append(file_id.rsplit('/', 1)[-1].rsplit('.', 1)[0])
    return file_names_array, file_ids_array
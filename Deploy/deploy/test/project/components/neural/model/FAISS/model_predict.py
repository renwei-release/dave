# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.07.28.
# 把一堆向量自动聚类，聚类个数由用户设置。和ScaNN是类似功能。
# faiss的代码来自以下链接：
# https://github.com/facebookresearch/faiss
#
# 运行容器环境cuda11
#
# ================================================================================
#
import faiss
from components.neural.model.Keras.model_predict import predict as keras_predict
from components.neural.model.DeepLab.model_predict import predict as deeplab_predict
from components.neural.model.MediaPipe.model_predict import predict as pose_predict
from components.neural.neural_tools.picture.picture_feature import t_picture_feature_ids_to_file_ids
from public.tools import *


def _load_feature_function(model_name):
    if model_name == None or model_name == 'vgg16':
        return keras_predict()
    elif model_name == 'deeplab':
        return deeplab_predict()
    elif model_name == 'pose':
        return pose_predict()
    else:
        return keras_predict()


def _model_predict(index, feature_ids, image_feature, top):
    image_feature.resize((1, image_feature.shape[0]))
    score_array, id_array = index.search(image_feature, top)
    score_array.resize((score_array.shape[1]))
    id_array.resize((id_array.shape[1]))
    file_names_array, file_ids_array = t_picture_feature_ids_to_file_ids(id_array, feature_ids)
    return file_names_array, file_ids_array, score_array


# =====================================================================


class predict():
    feature_function = None
    feature_ids = None
    index = None

    def __init__(self, model_path=None, model_name=None):
        if model_path == None:
            model_path = '/project/model/predict_model/FAISS/faiss_PhotographicAesthetics_train_20210809230018'
        model_file = model_path + '/model.index'
        ids_file = model_path + '/ids.idtable'
        self.feature_function = _load_feature_function(model_name)
        self.feature_ids = t_dict_load(ids_file)
        self.index = faiss.read_index(model_file)
        return

    def predict(self, image_info, top=10):
        return _model_predict(self.index, self.feature_ids, self.feature_function.feature(image_info), top)
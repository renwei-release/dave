# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.08.04.
# 把一堆向量自动聚类，聚类个数由用户设置。和FAISS是类似功能。
# 源代码来自：
# https://github.com/google-research/google-research/scann
# 运行容器环境cuda11
#
# ================================================================================
#
import scann
import numpy as np
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


def _model_predict(searcher, feature_ids, image_feature, top, TH):  #TH 置信度
    if len(image_feature) == 0:
        return None, None, None

    normalized_feature = image_feature / np.linalg.norm(image_feature)

    neighbors, score_array = searcher.search(q=normalized_feature, final_num_neighbors=top)
    file_names_array, file_ids_array = t_picture_feature_ids_to_file_ids(neighbors, feature_ids)

    for i,data in enumerate(score_array,):
        if data < TH:
            file_names_array = file_names_array[0:i]
            file_ids_array = file_ids_array[0:i]
            score_array = score_array[0:i]
            break

    return file_names_array, file_ids_array, score_array


# =====================================================================


class predict():
    feature_function = None
    feature_ids = None
    searcher = None

    def __init__(self, model_path=None, model_name=None):
        if model_path == None:
            model_path = '/project/model/predict_model/aesthetics/20211203092556/scann_PhotographicAesthetics_total_pose_121534_20211207092213'
        if model_name == None:
            model_name = 'pose'

        ids_file = model_path + '/ids.idtable'
        self.feature_function = _load_feature_function(model_name)
        self.feature_ids = t_dict_load(ids_file)
        self.searcher = scann.scann_ops_pybind.load_searcher(model_path)
        return

    #
    # 留意目前top不能超过100这个值，ScaNN内部做了限制。
    #
    def predict(self, image_info, top = 10, TH = 0.5, feature = None):
        if feature is None:
            feature = self.feature_function.feature(image_info=image_info)
        return _model_predict(self.searcher, self.feature_ids, feature, top, TH)
# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.08.03.
# scann的代码来自一下链接：
# https://github.com/google-research/google-research
# 由google-research可以一次性得到google的很多开源代码，其中就有scann
#
# 运行容器环境cuda11
#
# ================================================================================
#
import numpy as np
import scann
import sys
import traceback
from components.neural.neural_tools.picture.picture_feature import t_picture_feature_array
from public.tools import *


def _correction_number_of_cluster_centers(number_of_cluster_centers, file_number):
    if file_number < number_of_cluster_centers:
        number_of_cluster_centers = int(file_number / 16)
    if number_of_cluster_centers <= 0:
        number_of_cluster_centers = 1
    print(f'the scann number_of_cluster_centers:{number_of_cluster_centers}')
    return number_of_cluster_centers


def _model_train(dataset, number_of_cluster_centers, file_number):
    number_of_cluster_centers = _correction_number_of_cluster_centers(number_of_cluster_centers, file_number)

    normalized_dataset = dataset / np.linalg.norm(dataset, axis=1)[:, np.newaxis]
    # configure ScaNN as a tree - asymmetric hash hybrid with reordering
    # anisotropic quantization as described in the paper; see README

    try:
        # use scann.scann_ops.build() to instead create a TensorFlow-compatible searcher
        searcher = scann.scann_ops_pybind.builder(normalized_dataset, 10, "dot_product").tree(
            num_leaves=number_of_cluster_centers, num_leaves_to_search=100, training_sample_size=250000).score_ah(
            2, anisotropic_quantization_threshold=0.2).reorder(100).build()
    except:
        traceback.print_exc()
        searcher = None
    return searcher


def _save_model(searcher, model_path):
    if searcher != None:
        searcher.serialize(model_path)
    else:
        print(f'searcher is None!')
    return


def _save_ids(feature_ids, model_path):
    ids_file = model_path + '/ids.idtable'
    t_dict_save(ids_file, feature_ids)
    return 


def _save_feature(feature_ids, feature_array, model_path):
    feature_and_ids_array = {}

    for feature_index, feature_data in enumerate(feature_array):
        file_id = feature_ids[feature_index].rsplit('/', 1)[-1].rsplit('.', 1)[0]
        feature_data = feature_data.tolist()
        feature_and_ids_array[file_id] = feature_data

    ids_file = model_path + '/feature.idtable'
    t_dict_save(ids_file, feature_and_ids_array)
    return


def _save_result(searcher, feature_ids, feature_array, model_path):
    _save_model(searcher, model_path)
    _save_ids(feature_ids, model_path)
    _save_feature(feature_ids, feature_array, model_path)
    return


# =====================================================================


def train(
        train_path = '/project/dataset/Private/renwei/portrait',
        feature_model = 'pose'
    ):
    number_of_cluster_centers = 1000

    file_number, feature_array, feature_ids, feature_dimension, feature_name = t_picture_feature_array(train_path, feature_model)
    if file_number != None:
        model_path = t_train_path_to_model_path(train_path, 'scann', None, None, feature_name+'_'+str(file_number))
        searcher = _model_train(feature_array, number_of_cluster_centers, file_number)
        _save_result(searcher, feature_ids, feature_array, model_path)
        print(f'model_path:{model_path} feature_array:{len(feature_array)}')
    else:
        model_path = None
        print(f'invalid train_path:{train_path}')
    return model_path


if __name__ == "__main__":
    if len(sys.argv) <= 1:
        train()
    else:
        train(train_path=sys.argv[1], feature_model=sys.argv[2])
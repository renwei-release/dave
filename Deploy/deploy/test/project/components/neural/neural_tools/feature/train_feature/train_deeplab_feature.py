# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.09.22.
#
# 运行容器环境cuda11
#
# ================================================================================
#
import json
from public.tools import *
from components.neural.model.DeepLab.model_predict import predict as deeplab_predict


def _deeplab_feature(dataset_path):
    file_list, file_num = t_path_file_list(dataset_path)
    progress_bar = t_print_progress_bar(file_num)
    deeplab = deeplab_predict()

    deeplab_array = {}

    for file_name in file_list:
        file_id = file_name.rsplit('/', 1)[-1].rsplit('.', 1)[0]

        deeplab_feature = deeplab.feature(image_info=file_name, feature_dimension=151)
        deeplab_array[file_id] = deeplab_feature
    
        progress_bar.show()
    return deeplab_array, file_num


def _train_deeplab_feature(dataset_path):
    deeplab_array, file_num = _deeplab_feature(dataset_path)

    deeplab_path = t_train_path_to_model_path(dataset_path, 'deeplab_json', None, None, str(file_num))

    with open(deeplab_path+'/data.json', 'w') as f:
        json.dump(deeplab_array, f, indent=4)

    return deeplab_path


# =====================================================================


def train_deeplab_feature(dataset_path):
    return _train_deeplab_feature(dataset_path)
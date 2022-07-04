# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Yangyunchong All rights reserved.
# --------------------------------------------------------------------------------
# 2021.11.17.
#
# 运行容器环境cuda11
#
# ================================================================================
#
import json
from public.tools import *
from components.neural.model.Semseg.model_predict import predict as semseg_predict


def _semseg_feature(dataset_path):
    file_list, file_num = t_path_file_list(dataset_path)
    progress_bar = t_print_progress_bar(file_num)
    semseg = semseg_predict()

    semseg_array = {}

    for file_name in file_list:
        file_id = file_name.rsplit('/', 1)[-1].rsplit('.', 1)[0]

        semseg_feature = semseg.feature(image_info=file_name)
        semseg_array[file_id] = semseg_feature
    
        progress_bar.show()
    return semseg_array, file_num


def _train_semseg_feature(dataset_path):
    semseg_array, file_num = _semseg_feature(dataset_path)

    semseg_path = t_train_path_to_model_path(dataset_path, 'semseg_json', None, None, str(file_num))

    with open(semseg_path+'/data.json', 'w') as f:
        json.dump(semseg_array, f, indent=4)

    return semseg_path


# =====================================================================


def train_semseg_feature(dataset_path):
    return _train_semseg_feature(dataset_path)
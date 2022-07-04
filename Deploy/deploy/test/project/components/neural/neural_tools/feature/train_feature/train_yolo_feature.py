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
from components.neural.model.YOLO.model_predict import predict as yolo_predict


def _yolo_feature(dataset_path):
    file_list, file_num = t_path_file_list(dataset_path)
    progress_bar = t_print_progress_bar(file_num)
    yolo = yolo_predict()

    yolo_array = {}

    for file_name in file_list:
        file_id = file_name.rsplit('/', 1)[-1].rsplit('.', 1)[0]

        yolo_feature = yolo.feature(image_info=file_name)
        yolo_array[file_id] = yolo_feature
    
        progress_bar.show()
    return yolo_array, file_num


def _train_yolo_feature(dataset_path):
    yolo_array, file_num = _yolo_feature(dataset_path)

    yolo_path = t_train_path_to_model_path(dataset_path, 'yolo_json', None, None, str(file_num))

    with open(yolo_path+'/data.json', 'w') as f:
        json.dump(yolo_array, f, indent=4)

    return yolo_path


# =====================================================================


def train_yolo_feature(dataset_path):
    return _train_yolo_feature(dataset_path)
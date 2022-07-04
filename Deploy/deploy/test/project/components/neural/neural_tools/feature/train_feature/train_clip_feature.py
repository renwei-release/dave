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
from components.neural.model.CLIP.model_predict import predict as clip_predict


def _clip_feature(dataset_path):
    file_list, file_num = t_path_file_list(dataset_path)
    progress_bar = t_print_progress_bar(file_num)
    clip = clip_predict()
    clip_array = {}

    for file_name in file_list:
        file_id = file_name.rsplit('/', 1)[-1].rsplit('.', 1)[0]

        clip_feature = clip.feature(image_info=file_name)
        clip_array[file_id] = clip_feature
    
        progress_bar.show()
    return clip_array, file_num


def _train_clip_feature(dataset_path):
    clip_array, file_num = _clip_feature(dataset_path)
    clip_path = t_train_path_to_model_path(dataset_path, 'clip_json', None, None, str(file_num))

    with open(clip_path+'/data.json', 'w') as f:
        json.dump(clip_array, f, indent=4)

    return clip_path


# =====================================================================


def train_clip_feature(dataset_path):
    return _train_clip_feature(dataset_path)
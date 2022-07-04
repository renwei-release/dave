# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Yangyunchong All rights reserved.
# --------------------------------------------------------------------------------
# 2021.11.30.
#
# 运行容器环境cuda11
#
# ================================================================================
#
import json
import cv2
import sys
from public.tools import *


def _attribute_feature(dataset_path):
    file_list, file_num = t_path_file_list(dataset_path)
    progress_bar = t_print_progress_bar(file_num)
    attribute_array = {}

    for file_name in file_list:
        file_id = file_name.rsplit('/', 1)[-1].rsplit('.', 1)[0]
        image_pic = cv2.imread(file_name)
        attribute_feature = {'path':file_name,'width':image_pic.shape[1],'height':image_pic.shape[0]}
        attribute_array[file_id] = attribute_feature
        progress_bar.show()
    return attribute_array, file_num


def _train_attribute_feature(dataset_path):
    attribute_array, file_num = _attribute_feature(dataset_path)
    attribute_path = t_train_path_to_model_path(dataset_path, 'attribute_json', None, None, str(file_num))

    with open(attribute_path+'/data.json', 'w') as f:
        json.dump(attribute_array, f, indent=4, ensure_ascii=False)

    return attribute_path


# =====================================================================


def train_attribute_feature(dataset_path):
    return _train_attribute_feature(dataset_path)


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        dataset_path = sys.argv[1]
    else: 
        dataset_path = '/project/dataset/Private/PhotographicAesthetics/total'
    attribute_path = train_attribute_feature(dataset_path)
    print(f'dataset_path:{dataset_path} attribute_path:{attribute_path}')
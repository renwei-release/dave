# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.08.18.
#
# 运行容器环境cuda10 与 cuda11
#
# ================================================================================
#
from public.tools import *
from DeepLab.model_predict import predict as deeplab_predict


def _predict(deeplab, file_list):
    for file_name in file_list:
        resized_im, seg_map = deeplab.predict(image_info=file_name)
        image_id = file_name.rsplit('/', 1)[-1].rsplit('.', 1)[0]
        print(f'image_id:{image_id} seg_map:{seg_map.shape}')
    return


def _feature(deeplab, file_list):
    for file_name in file_list:
        feature_array = deeplab.feature(image_info=file_name)
        image_id = file_name.rsplit('/', 1)[-1].rsplit('.', 1)[0]
        print(f'image_id:{image_id} feature_array:{len(feature_array)}')
    return


# =====================================================================


def test(
        test_path='/project/dataset/Private/renwei/deeplab',
    ):
    deeplab = deeplab_predict()
    file_list, file_number = t_path_file_list(test_path)

    _predict(deeplab, file_list)
    _feature(deeplab, file_list)
    return


if __name__ == "__main__":
    test()
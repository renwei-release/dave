# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.09.13.
#
# 运行容器环境cuda11
#
# ================================================================================
#
from public.tools import *
from components.neural.model.MediaPipe.model_predict import predict as mediapipe_predict


def _feature(mediapipe, file_list):
    for file_name in file_list:
        feature_array = mediapipe.feature(image_info=file_name)
        image_id = file_name.rsplit('/', 1)[-1].rsplit('.', 1)[0]
        print(f'image_id:{image_id} feature_array:{len(feature_array)}')
    return


# =====================================================================


def test(
        test_path='/project/dataset/Private/renwei/mediapipe',
    ):
    mediapipe = mediapipe_predict()
    file_list, file_number = t_path_file_list(test_path)

    _feature(mediapipe, file_list)
    return


if __name__ == "__main__":
    test()
# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Yangyunchong All rights reserved.
# --------------------------------------------------------------------------------
# 2021.09.1.
#
# 运行容器环境cuda11
#
# ================================================================================
#
from model_predict import predict as semseg_predict
from public.tools import *


# =====================================================================


def test(
        test_path='/project/dataset/Private/PhotographicAesthetics/test',
    ):
    semseg = semseg_predict()
    file_list, file_number = t_path_file_list(test_path)

    for file_name in file_list:
        resized_im, seg_map = semseg.predict(image_info=file_name)
        image_id = file_name.rsplit('/', 1)[-1].rsplit('.', 1)[0]
        print(f"image_id:{image_id}")
    return


if __name__ == "__main__":
    test()
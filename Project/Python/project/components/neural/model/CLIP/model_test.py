# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Chengyuanquan All rights reserved.
# --------------------------------------------------------------------------------
# 2021.08.18.
# Clip模型代码来源于以下链接：
# copy from https://github.com/openai/CLIP
# 运行容器环境cuda11
# ================================================================================
#
from public.tools import *
from components.neural.model.CLIP.model_predict import predict as clip_predict


# =====================================================================


def test(
        test_path='/project/dataset/Private/PhotographicAesthetics/test',
    ):
    label = ['human', 'under water', 'animal']
    clip = clip_predict(label)

    file_list, file_number = t_path_file_list(test_path)

    for file_name in file_list:
        label, score = clip.predict(image_info=file_name)
        image_id = file_name.rsplit('/', 1)[-1].rsplit('.', 1)[0]
        print(f"image_id:{image_id} label:{label} score:{score}")
    return


if __name__ == "__main__":
    test()
# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Yangyunchong All rights reserved.
# --------------------------------------------------------------------------------
# 2021.09.10.
#
# 运行容器环境cuda11
#
# ================================================================================
#
from public.tools import *
from model_predict import predict as assessment_predict


# =====================================================================


def test(
        test_path='/project/dataset/Private/renwei'
    ):
    assessment = assessment_predict()
    file_list, file_number = t_path_file_list(test_path)

    for file_name in file_list:
        score = assessment.predict(image_info=file_name)
        image_id = file_name.rsplit('/', 1)[-1].rsplit('.', 1)[0]
        print(f"image_id:{image_id} score:{score}")
    return


if __name__ == "__main__":
    test()
# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 ChengYq All rights reserved.
# --------------------------------------------------------------------------------
# 2021.06.21.
#
# 运行容器环境cuda10,cudnn7
#
# ================================================================================
#
from public.tools import *
from model_predict import predict as easyocr_predict


# =====================================================================


def test(
        test_path='/project/dataset/Private/chengyuanquan/wine_ocrimg',
    ):
    easyocr = easyocr_predict()
    file_list= t_path_file_list(test_path)
    file_list = list(file_list)[0]

    for file_name in file_list:
        ocr_result = easyocr.predict(image_info=file_name)
        image_id = file_name.rsplit('/', 1)[-1].rsplit('.', 1)[0]
        print(f"image_id:{image_id} result:{ocr_result} ")
    return


if __name__ == "__main__":
    test()
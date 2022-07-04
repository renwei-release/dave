# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.06.21.
#
# 运行容器环境cuda11
#
# ================================================================================
#
from public.tools import *
from FAISS.model_predict import predict as faiss_predict


# =====================================================================


def test(
        test_path='/project/dataset/Private/PhotographicAesthetics/test',
    ):
    faiss = faiss_predict()
    file_list, file_number = t_path_file_list(test_path)

    for file_name in file_list:
        name_array, id_array, score_array = faiss.predict(image_info=file_name)
        image_id = file_name.rsplit('/', 1)[-1].rsplit('.', 1)[0]
        print(f"image_id:{image_id} id_array:{id_array} score_array:{score_array}")
    return


if __name__ == "__main__":
    test()
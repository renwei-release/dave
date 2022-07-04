#!/usr/bin/python
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.12.27.
# ================================================================================
#
import os
import sys
from components.neural.model.FAISS.model_predict import predict
from public.tools import *


def _picture_find_similar_image(file_names_array, score_array, file_name, similar_number, similar_array):
    for index, score in enumerate(score_array):
        if score >= 0.95:
            if file_name != file_names_array[index]:
                image_id = file_name.rsplit('/', 1)[-1].rsplit('.', 1)[0]
                similar_path = f'./picture_find_similar_image/{image_id}'

                os.system(f'mkdir -p {similar_path}')

                os.system(f'cp {file_name} {similar_path}')
                os.system(f'cp {file_names_array[index]} {similar_path}')

                similar_number += 1
                similar_array.append(file_name)
                similar_array.append(file_names_array[index])
    return similar_number, similar_array


# =====================================================================


def picture_find_similar_image(image_path, model_path):
    faiss_predict = predict(model_path, 'vgg16')

    file_list, file_number = t_path_file_list(image_path)
    similar_number = 0
    similar_array = []
    progress_bar = t_print_progress_bar(file_number)
    for file_name in file_list:
        if file_name not in similar_array:
            file_names_array, _, score_array = faiss_predict.predict(file_name)
            similar_number, similar_array = _picture_find_similar_image(file_names_array, score_array, file_name, similar_number, similar_array)
        progress_bar.show()
    print(f'file_number:{file_number} similar_number:{similar_number}')
    return


if __name__ == "__main__":
    if len(sys.argv) >= 3:
        image_path = sys.argv[1]
        model_path = sys.argv[2]
    else:
        image_path = '/project/dataset/Private/Painting'
        model_path = '/project/model/predict_model/aesthetics/20211218101050/faiss_Painting_vgg16_428594_20211218120901'
    picture_find_similar_image(image_path, model_path)
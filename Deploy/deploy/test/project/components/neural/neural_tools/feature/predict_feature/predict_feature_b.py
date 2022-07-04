# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Yangyunchong All rights reserved.
# --------------------------------------------------------------------------------
# 2021.11.19.
#
# 图像特征检索，  VGG特征 faiss向量检索 重排序
# 运行容器环境cuda11
#
# ================================================================================
#
import json
import os
import time
import numpy as np
from components.neural.model.FAISS.model_predict import predict as faiss_cluster
from components.neural.model.ScaNN.model_predict import predict as scann_cluster
from components.neural.model.Keras.model_predict import predict as keras_predict
from components.neural.model.MediaPipe.model_predict import predict as mediapipe_predict
from components.neural.model.CLIP.model_predict import predict as clip_predict
from components.neural.model.YOLO.model_predict import predict as yolo_predict
from sklearn.metrics.pairwise import cosine_similarity


def _predict_feature_model_init(vgg_model, pose_model, file_clip, file_yolo, file_attribute):
    if vgg_model == None:
        vgg_model = '/project/model/predict_model/aesthetics/20211217094923/faiss_PhotographicAesthetics_total_vgg16_129813_20211217110340'
    file_vgg = os.path.join(vgg_model, 'feature.idtable')
    if pose_model == None:
        pose_model = '/project/model/predict_model/aesthetics/20211217094923/scann_PhotographicAesthetics_total_pose_129813_20211218053851'
    file_pose = os.path.join(pose_model, 'feature.idtable')
    if file_clip == None:
        file_clip = '/project/model/predict_model/aesthetics/20211217094923/clip_json_PhotographicAesthetics_total_129813_20211221213328/data.json'
    if file_yolo == None:
        file_yolo = '/project/model/predict_model/aesthetics/20211217094923/yolo_json_PhotographicAesthetics_total_129813_20211217183345/data.json'
    if file_attribute == None:
        file_attribute = '/project/model/predict_model/aesthetics/20211217094923/attribute_json_PhotographicAesthetics_total_129813_20211218101045/data.json'
    return vgg_model, pose_model, file_vgg, file_pose, file_clip, file_yolo, file_attribute


def _predict_feature_cosine_similarity(model_id_array, total_feature_dict, total_feature_key, user_feature):
    model_score_array = []

    for model_id in model_id_array:
        if model_id in total_feature_key:
            id_score = cosine_similarity([total_feature_dict[model_id]], [user_feature]).tolist()[0][0]
        else:
            id_score = 0.00000000000000001
        model_score_array.append(id_score)

    rank = np.array([index for index, value in sorted(list(enumerate(model_score_array)), key=lambda x:x[1],reverse=True)])
    model_id_array = np.array(model_id_array)[rank].tolist()
    model_score_array.sort(reverse=True)

    return np.array(model_id_array), np.array(model_score_array)


def _predict_feature_get_dict(file):
    with open(file, 'r') as load_f:
        load_dict = json.load(load_f)
    dict_key = load_dict.keys()
    return load_dict, dict_key


def _predict_feature_vgg_cluster(vgg_cluster, dict_pose, key_pose, pose_feature, image_info, internal_top_number):
    _, vgg_id_array, vgg_score_array = vgg_cluster.predict(image_info=image_info, top=internal_top_number)
    if len(pose_feature) > 0:
        vgg_id_array, vgg_score_array = _predict_feature_cosine_similarity(vgg_id_array, dict_pose, key_pose, pose_feature)

    return np.array(vgg_id_array), np.array(vgg_score_array)


def _predict_feature_pose_cluster(pose_cluster, dict_vgg, key_vgg, vgg_feature, image_info, internal_top_number):
    _, pose_id_array, pose_score_array = pose_cluster.predict(image_info=image_info, top=internal_top_number, feature=None)
    if pose_id_array is not None:
        pose_id_array, pose_score_array = _predict_feature_cosine_similarity(pose_id_array, dict_vgg, key_vgg, vgg_feature)

    return np.array(pose_id_array), np.array(pose_score_array)


def _predict_feature_yolo_predict(yolo_predict, image_info):
    input_yolo = []
    people_bboxes = []
    file_size, bboxes, scores, classs = yolo_predict.predict(image_info=image_info)
    if bboxes != None:
        for index, score in enumerate(scores):
            if score > 0.6:
                input_yolo.append(classs[index])
            if classs[index] == 0:
                people_bboxes.append(bboxes[index])
    return input_yolo, people_bboxes


def _predict_feature_vgg_feature(vgg_predict, image_info):
    return vgg_predict.feature(image_info)


def _predict_feature_pose_feature(pose_predict, image_info):
    return pose_predict.feature(image_info)


def _predict_feature_clip_feature(clip_predict, image_info):
    return clip_predict.feature(image_info)


def _predict_feature_total(vgg_cluster, pose_cluster, \
    yolo_predict, vgg_predict, pose_predict, \
    dict_vgg, key_vgg, dict_pose, key_pose, \
    internal_top_number, \
    image_info):

    vgg_feature = _predict_feature_vgg_feature(vgg_predict, image_info)
    pose_feature = _predict_feature_pose_feature(pose_predict, image_info)

    vgg_id_array, vgg_score_array = _predict_feature_vgg_cluster(vgg_cluster, dict_pose, key_pose, pose_feature, image_info, internal_top_number)
    _, people_bboxes = _predict_feature_yolo_predict(yolo_predict, image_info)
    if len(people_bboxes) > 0:
        pose_id_array, pose_score_array = _predict_feature_pose_cluster(pose_cluster, dict_vgg, key_vgg, vgg_feature, image_info, internal_top_number)
    else:
        pose_id_array = np.empty([])
        pose_score_array = np.empty([])
    return vgg_id_array, vgg_score_array, pose_id_array, pose_score_array


def _predict_feature_find_new_id(find_id_array, new_id_array):
    not_in_index = 0

    while find_id_array[not_in_index] in new_id_array:
        if not_in_index + 1 < len(find_id_array):
            not_in_index += 1
        else:
            return None

    return find_id_array[not_in_index]


def _predict_feature_new_id(vgg_id_array, pose_id_array, internal_top_number):
    new_id_array = []

    if (vgg_id_array.shape) and (pose_id_array.shape):
        new_id_array = np.intersect1d(vgg_id_array, pose_id_array).tolist()
        intersection_find = 0

        for num in range(internal_top_number - len(new_id_array)):
            if intersection_find % 2 == 0:
                find_new_id = _predict_feature_find_new_id(vgg_id_array, new_id_array)
            else:
                find_new_id = _predict_feature_find_new_id(pose_id_array, new_id_array)
            if find_new_id != None:
                new_id_array.append(find_new_id)
            intersection_find += 1
    elif vgg_id_array.shape:
        new_id_array = vgg_id_array[0: internal_top_number]
    else:
        new_id_array = pose_id_array[0 : internal_top_number]

    return list(new_id_array)


def _predict_feature_tactics(vgg_id_array, vgg_score_array, pose_id_array, pose_score_array, dict_clip, key_clip, internal_top_number, clip_predict, image_info):
    new_id_array = _predict_feature_new_id(vgg_id_array, pose_id_array, internal_top_number)

    clip_user_feature = _predict_feature_clip_feature(clip_predict, image_info)

    new_id_array, new_score_array = _predict_feature_cosine_similarity(new_id_array, dict_clip, key_clip, clip_user_feature)

    return list(new_id_array), list(new_score_array)


def _predict_feature_output(approve_id_array, approve_score_array, load_dict_attribute, vgg_id_array, pose_id_array, topN):
    approve_attribute_array = []
    for approve_id in approve_id_array:
        if approve_id in load_dict_attribute:
            approve_attribute_array.append(load_dict_attribute[approve_id])
        else:
            approve_attribute_array.append({"width" : None, "height" : None, "path" : None})

    test_array = []
    if vgg_id_array.shape:
        vgg_id_array = vgg_id_array.tolist()
        for vgg_id in vgg_id_array:
            if vgg_id in load_dict_attribute:
                test_array.append(load_dict_attribute[vgg_id]['path'])
    if pose_id_array.shape:
        test_array.append(None)
        pose_id_array = pose_id_array.tolist()
        for pose_id in pose_id_array:
            if pose_id in load_dict_attribute:
                test_array.append(load_dict_attribute[pose_id]['path'])

    if len(test_array) != 0:
        return [ approve_id_array[0 : topN], approve_score_array[0 : topN], approve_attribute_array[0 : topN], test_array ]
    else:
        return [ approve_id_array[0 : topN], approve_score_array[0 : topN], approve_attribute_array[0 : topN] ]


# =====================================================================


class predict_feature():

    def __init__(self, vgg_model = None, pose_model = None, file_clip = None, file_yolo = None, file_attribute = None):

        self.vgg_model, self.pose_model, self.file_vgg, self.file_pose, self.file_clip, self.file_yolo, self.file_attribute = \
            _predict_feature_model_init(vgg_model, pose_model, file_clip, file_yolo, file_attribute)

        model_load_start_time = time.time()
        self.vgg_cluster = faiss_cluster(model_path=self.vgg_model, model_name='vgg16')
        self.pose_cluster = scann_cluster(model_path=self.pose_model, model_name='pose')
        self.vgg_predict = keras_predict()
        self.pose_predict = mediapipe_predict()
        self.clip_predict = clip_predict()
        self.yolo_predict = yolo_predict()
        model_load_end_time = time.time()
        print(f'model load time:{model_load_end_time - model_load_start_time} seconds')

        file_load_start_time = time.time()
        self.load_dict_vgg, self.dict_key_vgg = _predict_feature_get_dict(self.file_vgg)
        self.load_dict_pose, self.dict_key_pose = _predict_feature_get_dict(self.file_pose)
        self.load_dict_clip, self.dict_key_clip = _predict_feature_get_dict(self.file_clip)
        self.load_dict_attribute, self.dict_key_attribute = _predict_feature_get_dict(self.file_attribute)
        file_load_end_time = time.time()
        print(f'file load time:{file_load_end_time - file_load_start_time} seconds')

        self.internal_top_number = 20
        return

    def predict(self, image_info, topN = 9):

        vgg_id_array, vgg_score_array, pose_id_array, pose_score_array = \
            _predict_feature_total(self.vgg_cluster, self.pose_cluster, \
                self.yolo_predict, self.vgg_predict, self.pose_predict, \
                self.load_dict_vgg, self.dict_key_vgg, self.load_dict_pose, self.dict_key_pose, \
                self.internal_top_number, \
                image_info)

        approve_id_array, approve_score_array = \
            _predict_feature_tactics(vgg_id_array, vgg_score_array, \
                pose_id_array, pose_score_array, \
                self.load_dict_clip, self.dict_key_clip, \
                self.internal_top_number,
                self.clip_predict, image_info)

        return _predict_feature_output(approve_id_array, approve_score_array, self.load_dict_attribute, vgg_id_array, pose_id_array, topN)
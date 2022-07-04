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
import numpy as np
from components.neural.model.MediaPipe.model_predict import predict as pose_predict
from components.neural.model.ScaNN.model_predict import predict as scann_predict
from components.neural.model.YOLO.model_predict import predict as yolo_predict
from components.neural.model.FAISS.model_predict import predict as feature_predict
from components.neural.model.CLIP.model_predict import predict as clip_predict
from sklearn.metrics.pairwise import cosine_similarity

def _get_dict(file):
    with open(file, 'r') as load_f:
        load_dict = json.load(load_f)
    dict_key = load_dict.keys()
    return load_dict, dict_key

def _clip_is_people(featuer):
    if max(featuer[0],featuer[6],featuer[9],featuer[23],featuer[47],featuer[58],featuer[110]) >0:
        return True
    else:
        return False


# =====================================================================


class predict_feature():

    def __init__(self,vgg_model = None, pose_model= None, file_clip = None, file_yolo = None, file_attribute = None):

        if vgg_model == None:
            self.vgg_model = '/project/model/predict_model/aesthetics/20211217094923/faiss_PhotographicAesthetics_total_vgg16_129813_20211217110340'
        else:
            self.vgg_model = vgg_model
        if pose_model == None:
            self.pose_model = '/project/model/predict_model/aesthetics/20211217094923/scann_PhotographicAesthetics_total_pose_129813_20211218053851'
        else:
            self.pose_model = pose_model
        self.scann_mode = self.pose_model
        if file_yolo == None:
            self.file_yolo = '/project/model/predict_model/aesthetics/20211217094923/yolo_json_PhotographicAesthetics_total_129813_20211217183345/data.json'
        else:
            self.file_yolo = file_yolo
        if file_attribute == None:
            self.file_attribute = '/project/model/predict_model/aesthetics/20211217094923/attribute_json_PhotographicAesthetics_total_129813_20211218101045/data.json'
        else:
            self.file_attribute = file_attribute
        if file_clip == None:
            self.file_clip = '/project/model/predict_model/aesthetics/20211217094923/clip_json_PhotographicAesthetics_total_129813_20211221213328/data.json'
        else:
            self.file_clip = file_clip

        self.load_dict, self.dict_key = _get_dict(self.file_yolo)
        self.load_dict_attribute, self.dict_key_attribute = _get_dict(self.file_attribute)
        self.load_dict_clip, self.dict_key_clip = _get_dict(self.file_clip)
        self.clip_predict = clip_predict()
        self.vgg_predict = feature_predict(self.vgg_model, 'vgg16')
        self.scann_predict = scann_predict(model_path=self.scann_mode, model_name='pose')
        self.yolo_predict = yolo_predict(minimum_effective_score=0.5)
        self.pose_predict = pose_predict(yolo_fun=self.yolo_predict)

    def resort(self,name_array,score_array,id_array,topN):

        rank = np.array([index for index, value in sorted(list(enumerate(score_array)), key=lambda x:x[1],reverse=True)])
        id_array = np.array(id_array)[rank].tolist()
        name_array = np.array(name_array)[rank].tolist()
        score_array = score_array.tolist()
        score_array.sort(reverse=True)
        id_array = id_array[0:topN]
        score_array = score_array[0:topN]
        name_array = name_array[0:topN]

        return name_array,score_array,id_array

    def content_power(self,id_array):
        clip_feature = self.clip_feature
        for i, j in enumerate(clip_feature):
            if j < 0.05:
                self.clip_feature[i] = 0

        power_list = []
        for index, id in enumerate(id_array):
            if id in self.dict_key_clip:
                id_clip_feature = self.load_dict_clip[id]
                for i,j in enumerate(id_clip_feature):
                    if j < 0.05:
                        id_clip_feature[i] =0
                s_clip = cosine_similarity([id_clip_feature], [clip_feature])
                s_clip = s_clip / 5 + 0.8
            else:
                s_clip = 0.8
            power_list.append(s_clip)
        return power_list

    def has_person(self,pose_feature,image_info,topN):

        pose_name_array, pose_id_array, pose_score_array = self.scann_predict.predict(image_info=image_info, top=100,feature=pose_feature)
        power_list_content = self.content_power(pose_id_array)
        for index, id in enumerate(pose_id_array):
            pose_score_array[index] = pose_score_array[index] * power_list_content[index]
        pose_name_array,pose_score_array,pose_id_array = self.resort(pose_name_array,pose_score_array,pose_id_array,topN)

        return pose_name_array, pose_score_array, pose_id_array

    def no_person(self,image_info, input_yolo,topN):
        vgg_name_array, vgg_id_array, vgg_score_array = self.vgg_predict.predict(image_info=image_info, top=200)
        power_list_content = self.content_power(vgg_id_array)
        num_person = input_yolo.count(0)
        for index, id in enumerate(vgg_id_array):
            id_yolo = []
            if id in self.dict_key and self.load_dict[id] != None:
                for boxxs in self.load_dict[id]:
                    if boxxs[1] > 0.6:
                        id_yolo.append(boxxs[0])
            same = set(id_yolo) & set(input_yolo)
            if num_person == 0  or id_yolo.count(0) == num_person :
                power = 1
            else:
                power = 0.9

            for thing in same:
                input_count = input_yolo.count(thing)
                id_count = input_yolo.count(id_yolo)
                if input_count == id_count or input_count - id_count > 2 or id_count - input_count > 2:
                    power = power * 1.1
                else:
                    power = power * 1.05

            diff_power = 1
            for thing in set(id_yolo):
                if thing not in input_yolo:
                    if id_yolo.count(thing) > 1:
                        diff_power = diff_power * 0.8
                    else:
                        diff_power = diff_power * 0.9
            for thing in set(input_yolo):
                if thing not in id_yolo:
                    if input_yolo.count(thing) > 1:
                        diff_power = diff_power * 0.9
                    else:
                        diff_power = diff_power * 0.95
            if diff_power < 0.8:
                diff_power = 0.8
            vgg_score_array[index] = vgg_score_array[index] * power *diff_power * power_list_content[index]

        vgg_name_array, vgg_score_array, vgg_id_array = self.resort(vgg_name_array,vgg_score_array,vgg_id_array,topN)
        return vgg_name_array, vgg_score_array, vgg_id_array


    def predict(self, image_info, topN = 5):
        add_pose = False
        input_yolo = []
        people_bboxes = []
        file_size, bboxes, scores, classs = self.yolo_predict.predict(image_info=image_info)
        if bboxes != None:
            for index,score in enumerate(scores):
                if score > 0.6:
                    input_yolo.append(classs[index])
                if classs[index] ==0:
                    people_bboxes.append(bboxes[index])
        self.clip_feature = self.clip_predict.feature(image_info=image_info)

        if 0 in input_yolo :
            pose_feature = self.pose_predict.feature(image_info=image_info, feature_dimension=1024,bboxes = people_bboxes)
            if len(pose_feature)>0:
                pose_name_array, pose_score_array, pose_id_array = self.has_person(pose_feature,image_info,int(topN/2))
                add_pose = True
            if not _clip_is_people(self.clip_feature):
                while 0 in input_yolo:
                    input_yolo.remove(0)
        vgg_name_array, vgg_score_array, vgg_id_array = self.no_person(image_info, input_yolo, topN)
        if add_pose:
            for _index,_id in enumerate(pose_id_array) :
                if _id  not in vgg_id_array:
                    vgg_id_array[topN-_index - 1] =_id
                    vgg_score_array[topN-_index - 1] =  pose_score_array[_index]
                    vgg_name_array[topN-_index - 1] = pose_name_array[_index]

        result_size_array = []
        for id in vgg_id_array:
            if id in self.dict_key_attribute:
                result_size_array.append(self.load_dict_attribute[id])
            else:
                result_size_array.append({"width" : None, "height" : None})
        return vgg_id_array, vgg_score_array, result_size_array
# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.09.16. - 2021.11.17.
#
# 运行容器环境cuda11
#
# ================================================================================
#
import os
import sys 
from public.tools import *
from components.neural.third_party import *
from components.neural.third_party.tensorflow.tf_verno import tf_main_verno
if tf_main_verno() == '2':
    from components.neural.model.ScaNN.model_train import train as scann_train
from components.neural.model.FAISS.model_train import train as faiss_train
from .train_segsem_feature import train_semseg_feature
from .train_yolo_feature import train_yolo_feature
from .train_attribute_feature import train_attribute_feature
from .train_clip_feature import train_clip_feature

def _model_product_path(product_name):
    user_path = t_sys_model_user_path()
    current_time = t_time_current_str()
    return user_path + '/' + product_name + '/' + current_time


def _train_vgg16_feature(dataset_path):
    return faiss_train(dataset_path, 'vgg16')


def _train_pose_feature(dataset_path):
    return scann_train(dataset_path, 'pose')


def _train_semseg_feature(dataset_path):
    return train_semseg_feature(dataset_path)


def _train_yolo_feature(dataset_path):
    return train_yolo_feature(dataset_path)

def _train_clip_feature(dataset_path):
    return train_clip_feature(dataset_path)

def _train_attribute_feature(dataset_path):
    return train_attribute_feature(dataset_path)


def _train_model_all(dataset_path):
    vgg16_feature_path = _train_vgg16_feature(dataset_path)
    yolo_feature_path = _train_yolo_feature(dataset_path)
    pose_feature_path = _train_pose_feature(dataset_path)
    semseg_feature_path = _train_semseg_feature(dataset_path)
    clip_feature_path = _train_clip_feature(dataset_path)
    attribute_feature_path = _train_attribute_feature(dataset_path)
    return vgg16_feature_path, pose_feature_path, semseg_feature_path, yolo_feature_path, clip_feature_path, attribute_feature_path


def _move_model_to_product_path(
        model_product_path,
        vgg16_feature_path,
        pose_feature_path,
        semseg_feature_path,
        yolo_feature_path,
        clip_feature_path,
        attribute_feature_path):

    t_creat_path(model_product_path)

    if vgg16_feature_path != None:
        os.system(f'mv {vgg16_feature_path} {model_product_path}')
    if pose_feature_path != None:
        os.system(f'mv {pose_feature_path} {model_product_path}')
    if semseg_feature_path != None:
        os.system(f'mv {semseg_feature_path} {model_product_path}')
    if yolo_feature_path != None:
        os.system(f'mv {yolo_feature_path} {model_product_path}')
    if clip_feature_path != None:
        os.system(f'mv {clip_feature_path} {model_product_path}')
    if attribute_feature_path != None:
        os.system(f'mv {attribute_feature_path} {model_product_path}')
    return


# =====================================================================


def train_feature(product_name, dataset_path):
    tf_memory_adaptive()

    model_product_path = _model_product_path(product_name)

    print(f'dataset_path:{dataset_path} model_product_path:{model_product_path}')

    vgg16_feature_path, pose_feature_path, semseg_feature_path, yolo_feature_path,clip_feature_path, attribute_feature_path = _train_model_all(dataset_path)

    _move_model_to_product_path(
        model_product_path,
        vgg16_feature_path,
        pose_feature_path,
        semseg_feature_path,
        yolo_feature_path,
        clip_feature_path,
        attribute_feature_path)

    return model_product_path


if __name__ == "__main__":
    model_product_path = train_feature('test', '/project/dataset/Private/PhotographicAesthetics/test')
    print(f'model_product_path:{model_product_path}')
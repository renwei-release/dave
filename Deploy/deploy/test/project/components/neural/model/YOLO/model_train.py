# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.06.28.
# yolov5-5.0的代码来自一下链接：
# https://github.com/ultralytics/yolov5/tags
#
# 运行容器环境cuda11
#
# ================================================================================
#
import os
import torch
from public.tools import *


def _setup_database(data_path, train_path, validation_path, num_classes):
    t_file_replace_line(data_path, 'train: ', 'train: '+train_path)
    t_file_replace_line(data_path, 'val: ', 'val: '+validation_path)
    t_file_replace_line(data_path, 'nc: ', 'nc: '+str(num_classes))
    return


def _setup_network(network_path, num_classes):
    t_file_replace_line(network_path, 'nc: ', 'nc: '+str(num_classes)+'  # number of classes')
    return


def _model_train(yaml_file, model_type, model_path, model_pre_training, batch_size, epochs):
    os.system(f"cd yolov5 && python train.py --data {yaml_file} --cfg {model_type} --project {model_path} --weights '{model_pre_training}' --batch-size {batch_size} --epochs {epochs}")
    return


def _delete_cache_file(train_path, val_path):
    # 如果数据集路径有改变，一定要记得删除这个目录下的cache文件：
    # ./labels/train2017.cache
    # ./labels/val2017.cache
    # 他们记录了数据的路径，优先用cache里面的数据路径，相应处理代码大致是：
    # Read cache
    # cache.pop('hash')  # remove hash
    # cache.pop('version')  # remove version
    # labels, shapes, self.segments = zip(*cache.values())
    # self.labels = list(labels)
    # self.shapes = np.array(shapes, dtype=np.float64)
    # self.img_files = list(cache.keys())  # update
    train_path = train_path.split('/', 1)[-1]
    val_path = val_path.split('/', 1)[-1]

    train_name = train_path.rsplit('/', 1)[-1]
    val_name = val_path.rsplit('/', 1)[-1]

    train_label_cache_path = '/'+train_path.rsplit('/', 2)[0]+'/labels/'+train_name+'.cache'
    val_label_cache_path = '/'+val_path.rsplit('/', 2)[0]+'/labels/'+val_name+'.cache'

    print(f'clean cache file:{train_label_cache_path} {val_label_cache_path}')
    os.system(f"rm -rf {train_label_cache_path} {val_label_cache_path}")
    return


def _copy_result_file(network_file, train_path, validation_path, model_path):
    #
    # 以下是训练完成后，model里面保存文件的说明：
    # opt.yaml  训练参数设置
    # hyp.yaml 超参数的设置
    # results.txt 训练log保存
    # yolo.pt 为模型图与参数文件
    #
    model_name = network_file.rsplit('.', 1)[0]
    model_file = model_path + '/yolo.pt'
    os.system(f"mv {model_path}/exp/* {model_path} && rm -rf {model_path}/exp")
    os.system(f"mv {model_path}/weights/best.pt {model_file} && rm -rf {model_path}/weights")
    _delete_cache_file(train_path, validation_path)
    return model_file


def _load_result(result_file):
    result_data = open(result_file, 'r')
    precision_list = []
    recall_list = []
    for line_data in result_data.readlines():
        line_split = line_data.split()
        # 'metrics/precision', 'metrics/recall', 'metrics/mAP_0.5', 'metrics/mAP_0.5:0.95'
        # P, R, mAP@.5, mAP@.5-.95, val_loss(box, obj, cls)
        precision_list.append(line_split[2])
        recall_list.append(line_split[3])
    return max(precision_list), max(recall_list)


# =====================================================================


def train(
        train_path = '/project/dataset/Private/Sculpture/ObjectDetection/images/train2017',
        validation_path = '/project/dataset/Private/Sculpture/ObjectDetection/images/val2017',
        num_classes = 1,
        batch_size = 16,
        epochs = 300,
    ):
    model_root = './yolov5'
    data_file = 'sculpture.yaml'
    network_file = 'yolov5s.yaml'
    model_pre_training = ''     # 此处不使用预训练模型，而是重新开始训练
    model_path = t_train_path_to_model_path(train_path, network_file.rsplit('.', 1)[0], None, None, str(epochs))

    _setup_database(model_root+'/data/'+data_file, train_path, validation_path, num_classes)
    _setup_network(model_root+'/models/'+network_file, num_classes)    
    _model_train(data_file, network_file, model_path, model_pre_training, batch_size, epochs)
    model_file = _copy_result_file(network_file, train_path, validation_path, model_path)
    precision, recall = _load_result(model_path + '/results.txt')

    print(f"train_path:{train_path} validation_path:{validation_path}")
    print(f"model_file:{model_file}")
    print(f"precision:{precision} recall:{recall}")
    return model_file


if __name__ == "__main__":
    train()
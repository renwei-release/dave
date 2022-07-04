# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.08.13.
# DeepLab的代码来自一下链接：
# tensorflow 版本 https://github.com/tensorflow/models/tree/master/research/deeplab
#
# 运行容器环境cuda10
#
# 预训练模型下载来自：
# http://download.tensorflow.org/models/deeplabv3_xception_ade20k_train_2018_05_29.tar.gz
# https://github.com/tensorflow/models/blob/master/research/deeplab/g3doc/model_zoo.md
# ================================================================================
#
import os
import sys
from public.tools import *


def _model_train(train_path, dataset_name, pretrained_ckpt_path, model_path, batch_size, training_steps, input_size):
    os.system(f"\
        export PYTHONPATH=$PYTHONPATH:/project/dave/neural_network/model/DeepLab && \
        export PYTHONPATH=$PYTHONPATH:/usr/local/lib/python3.6/dist-packages/slim && \
        python deeplab/train.py \
        --logtostderr \
        --training_number_of_steps={training_steps} \
        --train_split=\"train\" \
        --model_variant=\"xception_65\" \
        --atrous_rates=6 \
        --atrous_rates=12 \
        --atrous_rates=18 \
        --output_stride=16 \
        --decoder_output_stride=4 \
        --train_crop_size=\"{input_size},{input_size}\" \
        --train_batch_size={batch_size} \
        --min_resize_value={input_size} \
        --max_resize_value={input_size} \
        --resize_factor=16 \
        --dataset={dataset_name} \
        --train_logdir={model_path} \
        --tf_initial_checkpoint={pretrained_ckpt_path} \
        --dataset_dir={train_path}")
    return


def _ckpt_to_pb(model_path, training_steps, num_classes, pb_model_name):
    os.system(f"\
        export PYTHONPATH=$PYTHONPATH:/project/dave/neural_network/model/DeepLab && \
        export PYTHONPATH=$PYTHONPATH:/usr/local/lib/python3.6/dist-packages/slim && \
        python deeplab/export_model.py \
        --logtostderr \
        --checkpoint_path={model_path}/model.ckpt-{training_steps} \
        --export_path={model_path}/{pb_model_name} \
        --model_variant='xception_65' \
        --atrous_rates=6 \
        --atrous_rates=12 \
        --atrous_rates=18 \
        --output_stride=16 \
        --decoder_output_stride=4 \
        --num_classes={num_classes} \
        --crop_size=1025 \
        --crop_size=2049  \
        --inference_scales=1.0")
    return


# =====================================================================


def train(
        train_path = '/project/dataset/Public/ADE20K/tfrecord',
        num_classes = 151,
        batch_size = 4,
        input_size = 513,
    ):
    dataset_name = "ade20k"
    pretrained_ckpt_path = '/project/model/pretrained_model/deeplabv3_xception_ade20k_train/model.ckpt'
    model_path = t_train_path_to_model_path(train_path, 'deeplabv3', None)
    training_steps = 150000
    pb_model_name = 'model.pb'

    _model_train(train_path, dataset_name, pretrained_ckpt_path, model_path, batch_size, training_steps, input_size)
    _ckpt_to_pb(model_path, training_steps, num_classes, pb_model_name)
    return


if __name__ == "__main__":
    train()
# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Yangyunchong All rights reserved.
# --------------------------------------------------------------------------------
# 2021.09.2.
# 运行容器环境cuda10 11
#
# ================================================================================
#
import os
from public.tools import *


def _model_train( model_path):
    os.system(f"\
        export PYTHONPATH=$PYTHONPATH:/project/dave/neural_network/model/Semseg && \
        python train.py \
        --cfg='/project/pip/semantic-segmentation-pytorch/semantic-segmentation-pytorch-master/config/ade20k-resnet18dilated-ppm_deepsup.yaml' \
        --train_logdir={model_path} \
        --gpus='0'")
    return


# =====================================================================


def train(
        train_path = '/project/dataset/Public/ADE20K/ADEChallengeData2016/images',
    ):
    model_path = t_train_path_to_model_path(train_path, 'resnet18dilated-ppm-deepsup_train', None)

    _model_train( model_path)
    return


if __name__ == "__main__":
    train()
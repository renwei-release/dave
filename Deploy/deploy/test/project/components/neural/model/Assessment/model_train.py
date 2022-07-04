# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Yangyunchong All rights reserved.
# --------------------------------------------------------------------------------
# 2021.09.13.
# 运行容器环境cuda10 11
#
# ================================================================================
#
import os
import sys
from public.tools import *


def _model_train(train_path, model_path):
    print(train_path, model_path)
    os.system(f"\
        export PYTHONPATH=$PYTHONPATH:/project/dave/neural_network/model/Assessment && \
        python train.py \
        --img_path={train_path} \
        --ckpt_path={model_path} \
        --train_csv_file=\"assessment/csv/train_labels.csv\" \
        --val_csv_file=\"assessment/csv/val_labels.csv\" \
        --train \
        ")
    return


# =====================================================================


def train(
        train_path = "/project/dataset/Public/AVA_dataset/images/images/",
    ):
    model_path = t_train_path_to_model_path(train_path, 'nima_train_vgg', None)

    _model_train( train_path,model_path)
    return


if __name__ == "__main__":
    train()
# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.06.28.
# Key.Net用来提取图像特征，类似SIFT：
# https://github.com/axelBarroso/Key.Net
#
# 运行容器环境cuda10
#
# ================================================================================
#
import os


def _model_train(train_path):
    os.system(f"cd KeyNet && python train_network.py --data-dir {train_path} --network-version KeyNet_default")
    return


# =====================================================================


def train(
        train_path = '/project/dataset/Public/imagenet-object-localization-challenge/ILSVRC/Data/CLS-LOC/train',
    ):
    _model_train(train_path)
    return


if __name__ == "__main__":
    train()
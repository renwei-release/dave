# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2023 Renwei All rights reserved.
# --------------------------------------------------------------------------------
#
# 运行容器环境cuda11
#
# ================================================================================
from components.neural.model.Similarity.Text2Vec.model_train import train as Text2Vec_train


# =====================================================================


def train(train_path = None, model_path = None, num_epochs = 1):
    return Text2Vec_train(train_path, model_path, num_epochs)


if __name__ == '__main__':
    train('./Text2Vec/dataset/JegoTrip', None, 1)
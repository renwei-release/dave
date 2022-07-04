# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2022 caojinglei All rights reserved.
# --------------------------------------------------------------------------------
# 2022.05.06.
# 运行容器环境cuda10
# 
# ================================================================================
from components.neural.model.VGG.model_predict import predict as vgg_predict
vgg_predict = vgg_predict()
def test():
    image_info = '/project/dataset/Private/Sculpture/Sculpture_test_data/louvre/0.jpg'
    sculpture_features_path = '/project/model/trained_model/cuda10-caojinglei-docker/Sculptures_feature'
    pca_path = sculpture_features_path + '/global_feature/vgg16_fc6_pca256.m'

    feat_norm = vgg_predict.predict(image_info)
    feat_pca = vgg_predict.feature(image_info, pca_path)  # feature pca
    print(feat_norm.shape, feat_pca.shape)
    return
if __name__ == "__main__":
    test()
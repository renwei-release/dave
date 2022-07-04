# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2022 caojinglei All rights reserved.
# --------------------------------------------------------------------------------
# 2022.05.06.
# 运行容器环境cuda10
# 全局特征提取：vgg-fc6，PCA到256维
# ================================================================================

import tensorflow as tf
from numpy import linalg as LA
import h5py
import cv2
import numpy as np
from sklearn.decomposition import PCA
import joblib
from components.neural.third_party.opencv import *
from components.neural.model.Keras.model_predict import _normalize

global sess, graph
############配置Session运行参数#####################
config = tf.ConfigProto()
config.gpu_options.allow_growth = True
sess = tf.Session(config=config)
graph = tf.get_default_graph()  # 获取计算图

def load_model_pb(model_file):
    with tf.gfile.FastGFile(model_file, 'rb') as f:
        graph_def = tf.GraphDef()
        graph_def.ParseFromString(f.read())
        sess.graph.as_default()
        tf.import_graph_def(graph_def, name='')# 导入计算图
def image_preprocessing(image_info): #image preprocessing
    img = opencv_open(image_info=image_info, BGRFLAG=1)
    img = cv2.resize(img, (224, 224))
    img = np.expand_dims(img, axis=0)
    return img

def feature_post_processing(feat): #feature post processing
    feat = feat.squeeze(axis=0)
    norm_feat = feat / LA.norm(feat)
    return norm_feat

def model_predict(image_info,input, output):  #feature extraction
    img = image_preprocessing(image_info)
    feat = sess.run(output, feed_dict={input: img})
    norm_feat = feature_post_processing(feat)
    return norm_feat

def feature_pca(features, d=128, whiten=True, copy=False, params=None):
    features = _normalize(features, copy=copy)
    if params:
        pca = params['pca']
        features = pca.transform(features)
    else:
        pca = PCA(n_components=d, whiten=whiten, copy=copy)
        features = pca.fit_transform(features)
        params = {'pca': pca}
    features = _normalize(features, copy=copy)
    return features, params

def PCA_predict(queryVec, params = None, pca_dim = None):
    if params is not None:
        queryVec = [queryVec]
     #   whitening_params = joblib.load(pca_path)
        whitening_params = params
        queryVec, _ = feature_pca(queryVec, params=whitening_params)
    else:
        queryVec, whitening_params = feature_pca(queryVec, d=pca_dim, copy=True)

    queryVec = np.squeeze(queryVec.astype(np.float32))
    return queryVec, whitening_params

class predict():
    model_pb = None
    input = None
    output = None
    pca_dim = None
    def __init__(self, model_path=None):
        if model_path == None:
            model_path = "/project/model/trained_model/cuda10-caojinglei-docker/Sculptures_train_tfrecords"
        self.model_pb =model_path + '/Sculpture_vgg16_model.pb'
        load_model_pb(self.model_pb)
        self.input = graph.get_tensor_by_name('input:0')
        self.output = graph.get_tensor_by_name('fc6/fc6:0')
        self.pca_dim = 256

    def predict(self, image_info=None):
        norm_feat = model_predict(image_info=image_info, input=self.input, output=self.output)
        return norm_feat
    def feature(self,image_info = None, params = None):
        normfeat = model_predict(image_info=image_info, input=self.input, output=self.output)
        feat_pca, _ = PCA_predict(normfeat, params=params)
        return feat_pca
    def feature_pca(self,normfeat = None):
        feat_pca, params = PCA_predict(normfeat, pca_dim=self.pca_dim)
        return feat_pca, params






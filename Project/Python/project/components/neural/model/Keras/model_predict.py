# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.06.07.
# 有很多Keras集成的模型，主要是VGG这种图像特征抽取的模型。
# 参考代码来自：
# https://tensorflow.google.cn/api_docs/python/tf/keras/applications
#
# 运行容器环境cuda11
#
# ================================================================================
#
import importlib
import io
import tensorflow as tf
import tensorflow.python.keras as keras
from PIL import Image as pil_image
from components.neural.model.Keras.model_table import model_table
from components.neural.third_party.tensorflow.tf_memory import tf_memory_adaptive
from tensorflow.keras import models
from tensorflow.keras.preprocessing.image import load_img, img_to_array
from components.neural.third_party.rmac import *
from public.tools import *


tf_memory_adaptive()


global graph, sess
graph = tf.compat.v1.get_default_graph()
sess = keras.backend.get_session()


MY_MODEL_FEATURE_DIMENSION = 1024

#
# 规定model_file是如下格式的字符串：
# '/model/mobilenet_v2_PhotographicAesthetics_2021748739/model.h5'
#
def _load_model_name(model_file):
    model_list = sorted(model_table, key=lambda i:len(i), reverse=True)
    for model_name in model_list:
        if model_file.find(model_name) >= 0:
            return model_name
    return 'vgg16'


def _load_model(model_file):
    model_name = _load_model_name(model_file)
    model_function = importlib.import_module(f'tensorflow.keras.applications.{model_table[model_name][0]}')
    preprocess_input = getattr(model_function, model_table[model_name][1])
    image_height = model_table[model_name][3]
    image_width = model_table[model_name][4]
    feature_level_name = model_table[model_name][5]
    classification_model = models.load_model(model_file)
    feature_model = models.Model(inputs=classification_model.input, outputs=classification_model.get_layer(feature_level_name).output)
    return preprocess_input, classification_model, feature_model, image_height, image_width, model_name


def load_img_bytes(bytes,  color_mode='rgb', target_size=None):
    img = pil_image.open(io.BytesIO(bytes))
    if color_mode == 'grayscale':
        if img.mode not in ('L', 'I;16', 'I'):
            img = img.convert('L')
    elif color_mode == 'rgba':
        if img.mode != 'RGBA':
            img = img.convert('RGBA')
    elif color_mode == 'rgb':
        if img.mode != 'RGB':
            img = img.convert('RGB')
    else:
        raise ValueError('color_mode must be "grayscale", "rgb", or "rgba"')

    if target_size is not None:
        width_height_tuple = (target_size[1], target_size[0])
        if img.size != width_height_tuple:
            img = img.resize(width_height_tuple, pil_image.NEAREST)
    return img


def _load_data(image_info, image_height, image_width):
    image_pic = None
    if type(image_info).__name__ == 'bytes':
        image = load_img_bytes(image_info, target_size=(image_height, image_width))
    elif type(image_info).__name__ == 'str':
        image = load_img(image_info, target_size=(image_height, image_width))
    return image


def _model_predict(image_info, image_height, image_width, model, preprocess_input):
    image = _load_data(image_info, image_height, image_width)
    img_array = img_to_array(image)
    img_array = np.expand_dims(img_array, 0)
    img_array = preprocess_input(img_array)
    predictions = model.predict(img_array)

    score = tf.nn.softmax(predictions[0])
    return np.argmax(score.eval(session=sess))


def _model_feature(image_info, image_height, image_width, model, preprocess_input):
    image = _load_data(image_info, image_height, image_width)
    img_array = img_to_array(image)
    img_array = np.expand_dims(img_array, 0)
    img_array = preprocess_input(img_array)
    features = model.predict(img_array)

    features = features.squeeze(axis = 0)
    features = features.transpose(2, 0, 1)
    # 通过RMAC把三维矩阵都统一变成一维[MY_MODEL_FEATURE_DIMENSION]
    features = apply_rmac_aggregation(features)
    features = normalize(features, copy=False)
    return features


# =====================================================================


class predict():
    preprocess_input = None
    classification_model = None
    feature_model = None
    image_height = 0
    image_width = 0
    _model_name_ = None

    def __init__(self, model_file=None):
        if model_file == None:
            model_file = '/project/model/predict_model/VGG/vgg16_PhotographicAesthetics_train_20210804172130/model.h5'
        with sess.as_default():
            with graph.as_default():
                self.preprocess_input, self.classification_model, self.feature_model, self.image_height, self.image_width, self._model_name_ = _load_model(model_file)

    def model_name(self):
        return self._model_name_

    def predict(self, image_info):
        return _model_predict(image_info, self.image_height, self.image_width, self.classification_model, self.preprocess_input)

    def feature(self, image_info):
        result = None
        with sess.as_default():
            with graph.as_default():
                result = _model_feature(image_info, self.image_height, self.image_width, self.feature_model, self.preprocess_input)
        return result
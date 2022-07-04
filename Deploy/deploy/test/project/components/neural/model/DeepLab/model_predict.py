# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.09.01.
# 图像的语义分割算法
# deeplab的代码来自一下链接：
# https://github.com/tensorflow/models/tree/master/research/deeplab
#
# 运行容器环境cuda10 与 cuda11
#
# ================================================================================
#
from components.neural.third_party.tensorflow.tf_verno import *
if tf_main_verno() == '1':
  import tensorflow as tf
else:
  import tensorflow.compat.v1 as tf
  tf.disable_v2_behavior()
import numpy as np
from public.tools import *
from components.neural.third_party.pillow import *
from tensorflow.python.platform import gfile
from PIL import Image


class DeepLabModel_TensorflowV1(object):
  INPUT_TENSOR_NAME = 'ImageTensor:0'
  OUTPUT_TENSOR_NAME = 'SemanticPredictions:0'
  input_size = 513

  def __init__(self, model_file):
    self.graph = tf.Graph()
    graph_def = tf.GraphDef.FromString(open(model_file, 'rb').read())
    if graph_def is None:
      raise RuntimeError('Cannot find inference graph in tar archive.')
    with self.graph.as_default():
      tf.import_graph_def(graph_def, name='')
    self.sess = tf.Session(graph=self.graph)

  def run(self, image):
    """Runs inference on a single image.
    Args:
      image: A PIL.Image object, raw input image.

    Returns:
      resized_image: RGB image resized from original input image.
      seg_map: Segmentation map of `resized_image`.
    """
    width, height = image.size
    resize_ratio = 1.0 * self.input_size / max(width, height)
    target_size = (int(resize_ratio * width), int(resize_ratio * height))
    resized_image = image.convert('RGB').resize(target_size, Image.ANTIALIAS)
    batch_seg_map = self.sess.run(
        self.OUTPUT_TENSOR_NAME,
        feed_dict={self.INPUT_TENSOR_NAME: [np.asarray(resized_image)]})

    seg_map = batch_seg_map[0]
    return resized_image, seg_map


def _DeepLabModel(model_file):
  return DeepLabModel_TensorflowV1(model_file)


def _feature_array(seg_map, feature_dimension, num_classes):
  if feature_dimension < num_classes:
    num_classes = feature_dimension
  feature_array = np.zeros((feature_dimension), dtype=float, order='C')
  total_point_number = seg_map.shape[0] * seg_map.shape[1]
  for feature_index in range(0, num_classes):
    feature_array[feature_index] = (np.sum(seg_map==feature_index)) / (total_point_number)
  return feature_array.tolist()


def _predict(model, image_info):
  original_im = pil_open(image_info)
  resized_im, seg_map = model(original_im)
  return resized_im, seg_map


def _feature(model, image_info, feature_dimension, num_classes):
  original_im = pil_open(image_info)
  resized_im, seg_map = model(original_im)
  return _feature_array(seg_map, feature_dimension, num_classes)


# =====================================================================


class predict():
  deeplab_model = None
  num_classes = None

  def __init__(self, model_file=None, num_classes=None):
    if model_file == None:
      model_file = '/project/model/predict_model/DeepLab/deeplabv3_ADE20K_tfrecord_20210819104159/model.pb'
      num_classes = 151
    self.num_classes = num_classes
    self.deeplab_model = _DeepLabModel(model_file)
    return

  def predict(self, image_info):
    return _predict(self.deeplab_model.run, image_info)

  def feature(self, image_info, feature_dimension=None):
    if feature_dimension == None:
      feature_dimension = 1024
    return _feature(self.deeplab_model.run, image_info, feature_dimension, self.num_classes)
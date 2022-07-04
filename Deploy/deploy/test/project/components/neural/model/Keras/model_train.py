# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.06.07.
# 参考代码来自：
# https://tensorflow.google.cn/api_docs/python/tf/keras/applications
#
# 运行容器环境cuda11
#
# ================================================================================
#
import importlib
import glob
import tensorflow as tf
from model_table import *
from tensorflow.keras.callbacks import ModelCheckpoint
from tensorflow.keras import optimizers
from tensorflow import keras
from tensorflow.keras import layers
from tensorflow.keras.layers import Dropout, Flatten, Dense
from tensorflow.keras.models import Sequential, Model
from tensorflow.keras.preprocessing.image import ImageDataGenerator
from public.tools import *
from components.neural.third_party.tensorflow import *


def _creat_pre_training_model(model_name):
    model_function = importlib.import_module(f'tensorflow.keras.applications.{model_table[model_name][0]}')
    preprocess_input = getattr(model_function, model_table[model_name][1])
    pre_training_model = getattr(model_function, model_table[model_name][2])
    image_height = model_table[model_name][3]
    image_width = model_table[model_name][4]
    # 定义模型
    pre_training_model = pre_training_model(weights="imagenet", include_top=False, input_shape=(image_width, image_height, 3))
    # 冻结预训练网络
    for layer in pre_training_model.layers[:]:
        layer.trainable = False
    return preprocess_input, pre_training_model, image_height, image_width


def _creat_classification_model(pre_training_model, train_path):
    # 自定义顶层网络
    classification_model = Sequential()
    # 将预训练网络展平
    classification_model.add(Flatten(input_shape=pre_training_model.output_shape[1:]))
    # 全连接层
    classification_model.add(Dense(1024, activation='relu'))
    classification_model.add(Dropout(0.5))
    classification_model.add(Dropout(0.5))
    classification_model.add(Dropout(0.5))
    # 输出层，多分类
    classification_model.add(Dense(len(glob.glob(train_path+'/*')), activation='softmax'))
    return classification_model


def _creat_model(model_name=None, train_path=None, learn_rate=0.0001, decay=0.0001):
    preprocess_input, pre_training_model, image_height, image_width = _creat_pre_training_model(model_name)
    classification_model = _creat_classification_model(pre_training_model, train_path)
    model = Model(inputs=pre_training_model.input, outputs=classification_model(pre_training_model.output))

    # 编译模型
    model.compile(loss='categorical_crossentropy',
              optimizer=optimizers.Adam(lr=learn_rate, decay=decay),
              metrics=['accuracy'])
    return preprocess_input, model, image_height, image_width


def _creat_dataset(preprocess_input, train_path, image_height, image_width, batch_size):
    return tf_load_dataset(
        train_path = train_path,
        preprocess_input = preprocess_input,
        image_height = image_height,
        image_width = image_width,
        batch_size = batch_size)


def _train_model(model_file, model, train_ds, val_ds, epochs):
    checkpointer = ModelCheckpoint(filepath=model_file,
                               monitor=tf_monitor(),
                               verbose=2,
                               save_best_only=True,
                               mode='max')

    history = model.fit(train_ds, epochs=epochs, validation_data=val_ds, callbacks=[checkpointer])
    return history


# =====================================================================


def train(
        train_path = '/project/dataset/Private/PhotographicAesthetics/total',
        model_name = 'vgg16',
        batch_size = 64,
        epochs = 100,
    ):
    preprocess_input, model, image_height, image_width = _creat_model(model_name=model_name, train_path=train_path)
    train_ds, val_ds, train_num, val_num = _creat_dataset(preprocess_input, train_path, image_height, image_width, batch_size)
    model_path = t_train_path_to_model_path(train_path, model_name, None)
    model_file = model_path + '/model.h5'
    history = _train_model(model_file, model, train_ds, val_ds, epochs)

    print(f"train dataset number:{train_num} val dataset number:{val_num}")
    print(f"acc:{max(history.history['accuracy'])} val_acc:{max(history.history['val_accuracy'])}")
    print(f"loss:{max(history.history['loss'])} val_loss:{max(history.history['val_loss'])}")
    return model_file


if __name__ == "__main__":
    train(model_name = 'vgg16', epochs = 1)
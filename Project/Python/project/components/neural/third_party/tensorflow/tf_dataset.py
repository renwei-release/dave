# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.06.07.
# ================================================================================
#
import tensorflow as tf
from tensorflow.keras import layers
from tensorflow.keras.preprocessing.image import ImageDataGenerator


validation_split = 0.1


def _tf_performance_dataset(train_ds, val_ds):
    AUTOTUNE = tf.data.AUTOTUNE
    train_ds = train_ds.cache().shuffle(64).prefetch(buffer_size=AUTOTUNE)
    val_ds = val_ds.cache().prefetch(buffer_size=AUTOTUNE)
    return train_ds, val_ds


def _tf_load_dataset_on_keras(train_path, image_height=224, image_width=224, batch_size=64):
    train_ds = tf.keras.preprocessing.image_dataset_from_directory(
        train_path,
        validation_split = validation_split,
        subset = "training",
        shuffle = True, seed = 123,
        image_size = (image_height, image_width),
        batch_size = batch_size)

    val_ds = tf.keras.preprocessing.image_dataset_from_directory(
        train_path,
        validation_split = validation_split,
        subset = "validation",
        shuffle = True, seed = 123,
        image_size = (image_height, image_width),
        batch_size = batch_size)

    train_num = len(train_ds.file_paths)
    val_num = len(val_ds.file_paths)

    return train_ds, val_ds, train_num, val_num


def _tf_load_dataset_on_imagegener(preprocess_input, train_path, image_height=224, image_width=224, batch_size=64):
    # 训练数据预处理器，随机水平翻转
    datagen = ImageDataGenerator(preprocessing_function = preprocess_input,
                horizontal_flip = True,
                validation_split = validation_split)

    # 训练数据生成器
    train_ds = datagen.flow_from_directory(train_path,
                            target_size = (image_height, image_width),
                            batch_size = batch_size,
                            class_mode = 'categorical',
                            shuffle = True,
                            subset = "training")
    # 验证数据生成器
    val_ds = datagen.flow_from_directory(train_path,
                            target_size = (image_height, image_width),
                            batch_size = batch_size,
                            class_mode = 'categorical',
                            shuffle = True,
                            subset = "validation")

    train_num = len(train_ds.filenames)
    val_num = len(val_ds.filenames)

    return train_ds, val_ds, train_num, val_num


# =====================================================================


def tf_load_dataset(train_path, preprocess_input=None, image_height=224, image_width=224, batch_size=64, dataset_performance=False):
    if preprocess_input == None:
        train_ds, val_ds, train_num, val_num = _tf_load_dataset_on_keras(train_path, image_height, image_width, batch_size)
    else:
        train_ds, val_ds, train_num, val_num = _tf_load_dataset_on_imagegener(preprocess_input, train_path, image_height, image_width, batch_size)

    if dataset_performance == True:
        train_ds, val_ds = _tf_performance_dataset(train_ds, val_ds)

    return train_ds, val_ds, train_num, val_num
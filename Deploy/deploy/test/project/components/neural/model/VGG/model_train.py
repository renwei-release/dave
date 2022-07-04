# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2022 caojinglei All rights reserved.
# --------------------------------------------------------------------------------
# 2022.05.06.
# 运行容器环境cuda10
# 
# ================================================================================
 
import tensorflow as tf 
import numpy as np 
import pdb
from datetime import datetime
from public.tools import *
import os.path
from vgg16.VGG16 import *
batch_size = 16
lr = 0.0001
n_cls = 1196
max_steps = 500000
 
def read_and_decode(filename):
    filename_queue = tf.train.string_input_producer([filename])
 
    reader = tf.TFRecordReader()
    _, serialized_example = reader.read(filename_queue)   
    features = tf.parse_single_example(serialized_example,
                                       features={
                                           'label': tf.FixedLenFeature([], tf.int64),
                                           'img_raw' : tf.FixedLenFeature([], tf.string),
                                       })
 
    img = tf.decode_raw(features['img_raw'], tf.uint8)
    img = tf.reshape(img, [224, 224, 3])
    img = tf.cast(img, tf.float32)# * (1. / 255)
    label = tf.cast(features['label'], tf.int64)
    return img, label
 
def train(data,mode_save_path):
    x = tf.placeholder(dtype=tf.float32, shape=[None, 224, 224, 3], name='input')
    y = tf.placeholder(dtype=tf.float32, shape=[None, n_cls], name='label')
    keep_prob = tf.placeholder(tf.float32)
    output = VGG_16(x, keep_prob, n_cls)
    print("output,label shape", output.shape, y.shape)
    loss = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(logits=output, labels=y))
    # train_step = tf.train.AdamOptimizer(learning_rate=0.1).minimize(loss)
    train_step = tf.train.GradientDescentOptimizer(learning_rate=lr).minimize(loss)

    accuracy = tf.reduce_mean(tf.cast(tf.equal(tf.argmax(output, 1), tf.argmax(y, 1)), tf.float32))

    images, labels = read_and_decode(data)
    img_batch, label_batch = tf.train.shuffle_batch([images, labels],
                                                    batch_size=batch_size,
                                                    capacity=392,
                                                    min_after_dequeue=200)
    label_batch = tf.one_hot(label_batch, n_cls, 1, 0)

    init = tf.global_variables_initializer()
    saver = tf.train.Saver()
    with tf.Session() as sess:
        sess.run(init)
        coord = tf.train.Coordinator()
        threads = tf.train.start_queue_runners(sess=sess, coord=coord)
        for i in range(max_steps):
            batch_x, batch_y = sess.run([img_batch, label_batch])
            _, loss_val = sess.run([train_step, loss], feed_dict={x: batch_x, y: batch_y, keep_prob: 0.8})
            if i % 100 == 0:
                train_arr = accuracy.eval(feed_dict={x: batch_x, y: batch_y, keep_prob: 1.0})
                print("%s: Step [%d]  Loss : %f, training accuracy :  %g" % (datetime.now(), i, loss_val, train_arr))
            if i % 1000 == 0:
                # checkpoint_path = os.path.join(FLAGS.train_dir, './model/model.ckpt')
                saver.save(sess, mode_save_path, global_step=i)
        coord.request_stop()
        coord.join(threads)
if __name__ == '__main__':
    data = '/project/dataset/Private/Sculpture/Globalfeature_train/train.tfrecords'
    save_path_model = '/project/model/trained_model/cuda10-caojinglei-docker/Sculptures_train_tfrecords'
    save_path_model = os.path.join(save_path_model, 'model.ckpt')
    print(save_path_model)
    train(data,save_path_model)

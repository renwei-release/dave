# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 yangyunchong All rights reserved.
# --------------------------------------------------------------------------------
# 2021.08.25.
# ================================================================================
#
import os
import sys
import tensorflow as tf
from tensorflow.python.framework import graph_util

def _ckpt_to_pb(ckpt_model_file, pb_model_file, output_node_names = None):
 
    reader = tf.compat.v1.train.NewCheckpointReader(ckpt_model_file)
    if output_node_names is not None:
        out_put = output_node_names.split(",")
    else:
        out_put = [key for key in reader.get_variable_to_shape_map()]
      
    with tf.compat.v1.Graph().as_default() as graph_old:
        isess = tf.compat.v1.InteractiveSession()

        isess.run(tf.compat.v1.global_variables_initializer())
        saver = tf.compat.v1.train.import_meta_graph(ckpt_model_file+'.meta', clear_devices=True)
        saver.restore(isess, ckpt_model_file)
        constant_graph = graph_util.convert_variables_to_constants(isess,
                                                                      isess.graph_def,
                                                                      out_put)
        constant_graph = graph_util.remove_training_nodes(constant_graph)

        with tf.compat.v1.gfile.GFile(pb_model_file, mode='wb') as f:
            f.write(constant_graph.SerializeToString())


# =====================================================================


def t_model_ckpt_to_pb(h5_model_file, pb_model_file, output_node_names=None):
    _ckpt_to_pb(h5_model_file, pb_model_file, output_node_names)
    return
 

# python ckpt_to_pb.py /project/dave/neural_network/model/DeepLab/model/deeplabv3_ADE20K_tfrecord_20210819104159/model.ckpt-150000 /project/dave/neural_network/model/DeepLab/model/deeplabv3_ADE20K_tfrecord_20210819104159/model.pb
# python ckpt_to_pb.py /project/model/trained_model/cuda10-caojinglei-docker/Sculptures_train_tfrecords/model.ckpt-495000 /project/model/trained_model/cuda10-caojinglei-docker/Sculptures_train_tfrecords/Sculpture_vgg16_model.pb fc6/fc6
if __name__ == "__main__":
    t_model_ckpt_to_pb(*sys.argv[1:])
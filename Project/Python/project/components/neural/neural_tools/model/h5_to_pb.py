# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.06.24.
# ================================================================================
#
import os
import sys
import tensorflow as tf
from tensorflow.python.framework.convert_to_constants import convert_variables_to_constants_v2


def _h5_to_pb(h5_model_file, pb_model_file):
    model = tf.keras.models.load_model(h5_model_file, compile=False)
    model.summary()

    full_model = tf.function(lambda Input: model(Input))
    full_model = full_model.get_concrete_function(tf.TensorSpec(model.inputs[0].shape, model.inputs[0].dtype))
 
    # Get frozen ConcreteFunction
    frozen_func = convert_variables_to_constants_v2(full_model)
    frozen_func.graph.as_graph_def()
 
    layers = [op.name for op in frozen_func.graph.get_operations()]
 
    # Save frozen graph from frozen ConcreteFunction to hard drive
    tf.io.write_graph(graph_or_graph_def=frozen_func.graph,
                      logdir="./frozen_models",
                      name=pb_model_file,
                      as_text=False)


# =====================================================================


def t_model_h5_to_pb(h5_model_file, pb_model_file):
    _h5_to_pb(h5_model_file, pb_model_file)
    return


# python h5_to_pb.py ./mobilenet_v2_PhotographicAesthetics_202162443356.h5 ./mobilenet_v2_PhotographicAesthetics_202162443356.pb
if __name__ == "__main__":
    t_model_h5_to_pb(*sys.argv[1:])
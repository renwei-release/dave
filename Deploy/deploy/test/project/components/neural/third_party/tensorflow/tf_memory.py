# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.06.07.
# ================================================================================
#
from .tf_verno import tf_main_verno
if tf_main_verno() == '1':
  import tensorflow as tf
else:
  import tensorflow.compat.v1 as tf
import keras


# =====================================================================


#
# 设置tensorflow占用显存的比例为自适应，默认tensorflow会占用90%多
#
def tf_memory_adaptive():
  tf_config = tf.ConfigProto()
  tf_config.gpu_options.allow_growth = True
  session = tf.Session(config=tf_config)
  keras.backend.set_session(session)
  return
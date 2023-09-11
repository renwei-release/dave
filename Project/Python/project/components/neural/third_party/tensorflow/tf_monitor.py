# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.06.07.
# ================================================================================
#
import tensorflow as tf
from .tf_verno import tf_main_verno


def tf_monitor():
    if tf_main_verno() == '1':
        monitor = 'val_acc'
    else:
        monitor = 'val_accuracy'
    return monitor
# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.07.14.
# ================================================================================
#
import cv2
from components.neural.third_party.opencv.opencv_image import *
import numpy as np


def _opencv_sift_root_features(kp, desc):
    if kp is not None:
        if len(kp) == 0:
            kp = None
            desc = None
        else:
            eps = 1e-7
            desc /= (desc.sum(axis=1, keepdims=True) + eps)  #
            desc = np.sqrt(desc)  #
    else:
        kp = None
        desc = None
    return kp, desc


def _opencv_sift_featuress(image_data, nfeatures, image_width, image_high):
    img = cv2.resize(image_data, (image_width, image_high))
    d = cv2.SIFT_create(nfeatures, contrastThreshold=1e-5)
    kp, desc = d.detectAndCompute(img, mask=np.ones(shape=img.shape[:-1] + (1,), dtype=np.uint8))
    kp, desc = _opencv_sift_root_features(kp, desc)
    pts = np.array([k.pt for k in kp], dtype=np.float32)
    ors = np.array([k.angle for k in kp], dtype=np.float32) # keypoint orientations in degrees
    scs = np.array([k.size for k in kp], dtype=np.float32)  # keypoint scales
    return pts, ors, scs, desc


# =====================================================================


def opencv_sift_features(image_info, nfeatures, image_width, image_high):
    image_data = opencv_open(image_info)
    return _opencv_sift_featuress(image_data, nfeatures, image_width, image_high)
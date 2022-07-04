# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.07.13.
# Key.Net用来提取图像特征，类似SIFT：
# https://github.com/cavalli1234/AdaLAM/archive/refs/heads/master.zip
#
# 运行容器环境cuda10 与 cuda11
#
# ================================================================================
#
from components.neural.third_party.numpy import *
from components.neural.third_party.opencv import *
from adalam import AdalamFilter


ENABLE_ADALAM_FEATURES_ON_OPENCV = True


def _load_keynet():
    if ENABLE_ADALAM_FEATURES_ON_OPENCV == False:
        from components.neural.model.KEYNET.model_predict import predict as keynet_model
        return keynet_model()
    else:
        return None


def _load_features(keynet, image_info, image_width, image_high):
    if keynet == None:
        nfeatures = 8000
        k, o, s, d = opencv_sift_features(image_info, nfeatures, image_width, image_high)
    else:
        k, d = keynet.predict(image_info, image_width, image_high)
        o = None
        s = None
    return k, o, s, d


def _adalam_match_on_features(
        k1, o1, s1, d1,
        k2, o2, s2, d2,
        image_width, image_high
    ):
    if numpy_tools_cmp_array(d1, d2) == True:
        return 9999999999999999

    imgsize = [image_width, image_high]

    custom_config = {}

    if (o1 is None) or (o2 is None):
        custom_config["orientation_difference_threshold"] = None
    if (s1 is None) or (s2 is None):
        custom_config["scale_rate_threshold"] = None
    
    if len(d2) < 5 or len(d1) < 5:
        inliners_num = 0
    else:
        matcher = AdalamFilter(custom_config=custom_config)

        matches = matcher.match_and_filter(
        k1=k1, k2=k2, o1=o1, o2=o2, d1=d1, d2=d2, s1=s1, s2=s2,
        im1shape=imgsize, im2shape=imgsize).cpu().numpy()
        inliners_num = len(matches)
    return inliners_num


# =====================================================================


class predict():
    image_width = 500
    image_high = 500
    keynet = None

    def __init__(self):
        self.image_width = 500
        self.image_high = 500
        self.keynet = _load_keynet()
        return

    def picture_features(self, image_info):
        k, o, s, d, = _load_features(self.keynet, image_info, self.image_width, self.image_high)
        return k, o, s, d

    def picture_similarity(self, image_1_info, image_2_info):
        k1, o1, s1, d1, = _load_features(self.keynet, image_1_info, self.image_width, self.image_high)
        k2, o2, s2, d2 = _load_features(self.keynet, image_2_info, self.image_width, self.image_high)
        return _adalam_match_on_features(k1, o1, s1, d1, k2, o2, s2, d2, self.image_width, self.image_high)

    def features_similarity(self, k1, o1, s1, d1, k2, o2, s2, d2):
        return _adalam_match_on_features(k1, o1, s1, d1, k2, o2, s2, d2, self.image_width, self.image_high)
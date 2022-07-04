# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.07.08.
# Key.Net用来提取图像特征，类似SIFT：
# https://github.com/axelBarroso/Key.Net
# 这里有一个有意思的事情，Key.Net在这个目录下Key.Net\keyNet\aux，有一个aux的文件夹
# 而aux是Windows保留字段，所以在Windows下不能有这个命名，这里为适应Windows修改了文件夹
# 命名和对应的代码
#
# 运行容器环境cuda10
#
# ================================================================================
#
import cv2
from components.neural.third_party.opencv.opencv_image import *
from components.neural.model.KEYNET.extract_multiscale_features import *
from components.neural.model.KEYNET.KeyNet.keyNet.model.hardnet_pytorch import *


def _image_preprocessing(image_info, image_width, image_high):
    image_data = opencv_open(image_info=image_info, BGRFLAG=1)
    image_data = cv2.resize(image_data, (image_width, image_high))
    image_data = image_data.reshape(image_data.shape[0], image_data.shape[1], 3)
    image_data = cv2.cvtColor(image_data, cv2.COLOR_BGR2GRAY)
    image_data = image_data.reshape(image_data.shape[0], image_data.shape[1], 1)
    image_data = image_data.astype(float) / image_data.max()
    return image_data


def _load_image(image_info, image_width, image_high):
    image_data = _image_preprocessing(image_info, image_width, image_high)
    return image_data


def _load_model(model_file):
    # Load model
    model = HardNet()
    checkpoint = torch.load(model_file)
    model.load_state_dict(checkpoint['state_dict'])
    model.eval()
    model.cuda()
    return model


# =====================================================================


class predict():
    model = None
    model_file = None
    checkpoint_file = None

    def __init__(self, model_path=None):
        if model_path == None:
            model_path = '/project/model/pretrained_model/KeyNet'
        self.model_file = model_path+'/HardNet++.pth'
        self.checkpoint_file = model_path+'/KeyNet_default'
        self.model = _load_model(self.model_file)

    def predict(self, image_info, image_width, image_high):
        image_data = _load_image(image_info, image_width, image_high)
        pts, desc = extract_multiscale_features(model=self.model, checkpoint_det_dir=self.checkpoint_file, image_data=image_data)
        # x, y, s, r to x, y
        pts = pts[:, 0:2]
        return pts, desc
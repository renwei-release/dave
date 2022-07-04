# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.07.15.
# 用来做目标跟踪。
# yolov5-5.0的代码来自一下链接：
# https://github.com/ultralytics/yolov5/tags
#
# 运行容器环境cuda11
#
# ================================================================================
#
import torch
from public.tools import *
t_sys_path_append('/yolov5')
from components.neural.model.YOLO.yolov5.utils.torch_utils import select_device
from components.neural.model.YOLO.yolov5.utils.datasets import letterbox
from components.neural.model.YOLO.yolov5.models.yolo import Model
from components.neural.model.YOLO.yolov5.utils.general import non_max_suppression,scale_coords
from components.neural.model.YOLO.yolov5.models.experimental import attempt_load
from components.neural.third_party.opencv import *


def _load_model_on_pt_and_yaml(model_file, yaml_file):
    #
    # 此时的model_file为权重文件
    # yaml_file为网络图相关文件
    # 
    device = select_device('cpu')
    print(f"model_file:{model_file} yaml_file:{yaml_file}")
    model = Model(yaml_file).to(device)
    model.load_state_dict(torch.load(model_file))
    model.eval()
    return model, device


def _load_model_on_pt(model_file):
    device = select_device('cpu')
    model = attempt_load(model_file, map_location=device)
    return model, device


def _load_model(model_path):
    model_file = model_path + '/yolo.pt'
    yaml_file = model_path + '/yolo.yaml'
    if t_path_or_file_exists(yaml_file) == True:
        return _load_model_on_pt_and_yaml(model_file, yaml_file)
    else:
        return _load_model_on_pt(model_file)


def _load_image(image_info, device):
    original_image = opencv_open(image_info=image_info, BGRFLAG=1)
    if original_image.all() == None:
        return None, None

    resize_img = letterbox(original_image, new_shape=640)[0]
    resize_img = resize_img[:, :, ::-1].transpose(2, 0, 1)          # BGR to RGB, to 3x640x640
    resize_img = np.ascontiguousarray(resize_img)

    resize_img = torch.from_numpy(resize_img).to(device)
    half = device.type != 'cpu'
    resize_img = resize_img.half() if half else resize_img.float()  # uint8 to fp16/32
    resize_img /= 255.0  # 0 - 255 to 0.0 - 1.0
    if resize_img.ndimension() == 3:
        resize_img = resize_img.unsqueeze(0)

    return resize_img, original_image


def _predict_model(model, minimum_effective_score, resize_img, original_image, user_want_class):
    # Inference
    pred = model(resize_img, augment=False)[0]
    # Apply NMS
    pred = non_max_suppression(pred, 0.4, 0.5, classes=None, agnostic=False)
    # Process detections
    det = pred[0]
    h, w, c = original_image.shape
    box_number = 0

    if det is not None and len(det):
        ret = scale_coords(resize_img.shape[2:], det[:, :4], original_image.shape).round()
        ret = ret.cpu().detach().numpy()
        ret = np.int16(ret)
        bboxes = []
        scores = []
        classs = []
        for index in range(len(ret)):
            score = det[index][4].tolist()
            if score > minimum_effective_score:
                # x1, y1, x2, y2
                yolo_bboxe = ret[index, :].tolist()
                yolo_class = int(det[index][5].tolist())

                if user_want_class == None or user_want_class == yolo_class:
                    bboxes.append(yolo_bboxe)
                    scores.append(score)
                    classs.append(yolo_class)
                    box_number += 1

    if box_number == 0:
        return [w, h], None, None, None
    else:
        return [w, h], bboxes, scores, classs


def _bbox_feature_model(w, h, bbox, cla, score):
    x_center_point = bbox[0] + ((bbox[2] - bbox[0]) / 2)
    y_center_point = bbox[1] + ((bbox[3] - bbox[1]) / 2)
    x_proportion = x_center_point / w
    y_proportion = y_center_point / h
    size_proportion = ((bbox[2] - bbox[0]) * (bbox[3] - bbox[1])) / (w * h)
    return [cla, score, x_proportion, y_proportion, size_proportion]


def _feature_model(file_size, bboxes, scores, classs):
    feature_array = []
    for bbox, cla, score in zip(bboxes, classs, scores):
        bbox_feature = _bbox_feature_model(file_size[0], file_size[1], bbox, cla, score)
        feature_array.append(bbox_feature)
    return feature_array


# =====================================================================


class predict():
    model = None
    device = None
    minimum_effective_score = 0.6

    def __init__(self, model_path=None, minimum_effective_score=0.6):
        if model_path == None:
            model_path = '/project/model/pretrained_model/YOLOv5/yolov5x'
        self.model, self.device = _load_model(model_path)
        self.minimum_effective_score = minimum_effective_score
        return

    #
    # user_want_class 用来填充用户希望获取的某一类目标信息
    # 比如在yolov5s预训练模型里面，是用的coco128数据集。
    # 通过参考coco128.yaml文件可以知道，person这个类型的id为0
    # 如果不填充，是获取所有分类的目标信息。
    #
    def predict(self, image_info=None, user_want_class=None):
        resize_img, original_image = _load_image(image_info, self.device)
        if (resize_img.all() == None) or (original_image.all() == None):
            return None, None, None, None
        return _predict_model(self.model, self.minimum_effective_score, resize_img, original_image, user_want_class)

    def feature(self, image_info=None, user_want_class=None):
        if image_info is None:
            return None
        file_size, bboxes, scores, classs = self.predict(image_info=image_info, user_want_class=user_want_class)
        if bboxes == None:
            return None
        return _feature_model(file_size, bboxes, scores, classs)
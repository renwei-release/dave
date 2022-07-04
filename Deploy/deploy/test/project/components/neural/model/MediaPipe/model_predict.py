# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.09.13.
# Google发布的人体特征抽取，可以抽取人脸，人体形态等。其中人脸的点数可以抽取468个。
# 运行容器环境cuda11
#
# ================================================================================
#
import mediapipe as mp
import numpy as np
from components.neural.model.YOLO.model_predict import predict as yolo_predict
from components.neural.third_party.opencv import *


def _the_distance(point_a, point_b):
    return point_b - point_a


def _the_slope(x, y):
    if y == 0:
        return 0.0
    return x/y


def _the_two_point_slope(pose_landmarks, point_a, point_b):
    if point_a == point_b:
        return 0.0
    if (pose_landmarks.landmark[point_a].x <= 0) or (pose_landmarks.landmark[point_a].x >= 1):
        return 0.0
    if (pose_landmarks.landmark[point_a].y <= 0) or (pose_landmarks.landmark[point_a].y >= 1):
        return 0.0
    if (pose_landmarks.landmark[point_b].x <= 0) or (pose_landmarks.landmark[point_b].x >= 1):
        return 0.0
    if (pose_landmarks.landmark[point_b].y <= 0) or (pose_landmarks.landmark[point_b].y >= 1):
        return 0.0

    x = _the_distance(pose_landmarks.landmark[point_a].x, pose_landmarks.landmark[point_b].x)            
    y = _the_distance(pose_landmarks.landmark[point_a].y, pose_landmarks.landmark[point_b].y)

    #
    # visibility：一个值，用于[0.0, 1.0]指示界标在图像中可见（存在且未被遮挡）的可能性。
    #
    return pose_landmarks.landmark[point_b].visibility + _the_slope(x, y)


def _the_sport_point_slope(point, pose_landmarks, feature_array, feature_dimension, feature_index):
    if (feature_index + 13) >= feature_dimension:
        return feature_index

    #00
    feature_array[feature_index] = _the_two_point_slope(pose_landmarks, point, mp.solutions.holistic.PoseLandmark.NOSE)
    feature_index += 1

    #07
    feature_array[feature_index] = _the_two_point_slope(pose_landmarks, point, mp.solutions.holistic.PoseLandmark.LEFT_EAR)
    feature_index += 1
    #08
    feature_array[feature_index] = _the_two_point_slope(pose_landmarks, point, mp.solutions.holistic.PoseLandmark.RIGHT_EAR)
    feature_index += 1

    #13
    feature_array[feature_index] = _the_two_point_slope(pose_landmarks, point, mp.solutions.holistic.PoseLandmark.LEFT_ELBOW)
    feature_index += 1
    #14
    feature_array[feature_index] = _the_two_point_slope(pose_landmarks, point, mp.solutions.holistic.PoseLandmark.RIGHT_ELBOW)
    feature_index += 1

    #15
    feature_array[feature_index] = _the_two_point_slope(pose_landmarks, point, mp.solutions.holistic.PoseLandmark.LEFT_WRIST)
    feature_index += 1
    #16
    feature_array[feature_index] = _the_two_point_slope(pose_landmarks, point, mp.solutions.holistic.PoseLandmark.RIGHT_WRIST)
    feature_index += 1

    #23
    feature_array[feature_index] = _the_two_point_slope(pose_landmarks, point, mp.solutions.holistic.PoseLandmark.LEFT_HIP)
    feature_index += 1
    #24
    feature_array[feature_index] = _the_two_point_slope(pose_landmarks, point, mp.solutions.holistic.PoseLandmark.RIGHT_HIP)
    feature_index += 1

    #25
    feature_array[feature_index] = _the_two_point_slope(pose_landmarks, point, mp.solutions.holistic.PoseLandmark.LEFT_KNEE)
    feature_index += 1
    #26
    feature_array[feature_index] = _the_two_point_slope(pose_landmarks, point, mp.solutions.holistic.PoseLandmark.RIGHT_KNEE)
    feature_index += 1

    #31
    feature_array[feature_index] = _the_two_point_slope(pose_landmarks, point, mp.solutions.holistic.PoseLandmark.RIGHT_FOOT_INDEX)
    feature_index += 1
    #32
    feature_array[feature_index] = _the_two_point_slope(pose_landmarks, point, mp.solutions.holistic.PoseLandmark.LEFT_FOOT_INDEX)
    feature_index += 1

    return feature_index


def _pose_result_to_feature(pose_landmarks, feature_array, feature_dimension, feature_index):
    #11
    feature_index = _the_sport_point_slope(mp.solutions.holistic.PoseLandmark.LEFT_SHOULDER, pose_landmarks, feature_array, feature_dimension, feature_index)
    #12
    feature_index = _the_sport_point_slope(mp.solutions.holistic.PoseLandmark.RIGHT_SHOULDER, pose_landmarks, feature_array, feature_dimension, feature_index)
    return feature_index


def _feature_pose(mp_pose_fun, bboxes, image_info, feature_dimension):
    image_data = opencv_open(image_info)
    if image_data.all() == None:
        return []

    if bboxes == None:
        return []

    h = image_data.shape[0]
    w = image_data.shape[1]

    feature_array = np.zeros((feature_dimension), dtype=float, order='C')
    feature_index = 0

    for bboxe in bboxes:
        size_proportion = ((bboxe[2] - bboxe[0]) * (bboxe[3] - bboxe[1])) / (w * h)
        if size_proportion < 0.05:
            continue
        image_bboxe_data = image_data[bboxe[1]:bboxe[3], bboxe[0]:bboxe[2]]
        pose_results = mp_pose_fun.process(image_bboxe_data)
        if (pose_results != None) and (pose_results.pose_landmarks != None):
            feature_index = _pose_result_to_feature(pose_results.pose_landmarks, feature_array, feature_dimension, feature_index)

    if (feature_index == 0) or (np.all(feature_array==0) == True):
        return []
    else:
        return feature_array


# =====================================================================


class predict():
    mp_pose_fun = None
    yolo_fun = None

    def __init__(self, yolo_fun=None):
        self.mp_pose_fun = mp.solutions.holistic.Holistic(static_image_mode=True, min_detection_confidence=0.55)

        #
        # 如果yolo已经在其他地方初始化过，那么在这里就不用初始化了。
        #
        if yolo_fun == None:
            self.yolo_fun = yolo_predict(minimum_effective_score=0.5)
        else:
            self.yolo_fun = yolo_fun
        return

    def feature(self, image_info, feature_dimension=None, bboxes=None):
        if feature_dimension == None:
            feature_dimension = 1024
        if bboxes == None:
            _, bboxes, _, _ = self.yolo_fun.predict(image_info=image_info, user_want_class=0)
        feature_array = _feature_pose(self.mp_pose_fun, bboxes, image_info, feature_dimension)
        return feature_array
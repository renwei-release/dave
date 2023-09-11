# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.06.23.
# ================================================================================
#


# 参考链接：https://keras.io/api/applications/
model_table = {
    "vgg16": ["vgg16", "preprocess_input", "VGG16", 224, 224, "block5_pool"],
    "vgg19": ["vgg19", "preprocess_input", "VGG19", 224, 224, ""],
    "resnet50": ["resnet", "preprocess_input", "ResNet50", 224, 224, ""],
    "resnet101": ["resnet", "preprocess_input", "ResNet101", 224, 224, ""],
    "resnet152": ["resnet", "preprocess_input", "ResNet152", 224, 224, ""],
    "resnet50v2": ["resnet_v2", "preprocess_input", "ResNet50V2", 224, 224, ""],
    "resnet101v2": ["resnet_v2", "preprocess_input", "ResNet101V2", 224, 224, ""],
    "resnet152v2": ["resnet_v2", "preprocess_input", "ResNet152V2", 224, 224, ""],
    "inception_v3": ["inception_v3", "preprocess_input", "InceptionV3", 299, 299, ""],
    "inception_resnet_v2": ["inception_resnet_v2", "preprocess_input", "InceptionResNetV2", 299, 299, ""],
    "mobilenet": ["mobilenet", "preprocess_input", "MobileNet", 244, 244, ""],
    "mobilenet_v2": ["mobilenet_v2", "preprocess_input", "MobileNetV2", 224, 224, ""],
    "densenet121": ["densenet", "preprocess_input", "DenseNet121", 224, 224, ""],
    "densenet169": ["densenet", "preprocess_input", "DenseNet169", 224, 224, ""],
    "densenet201": ["densenet", "preprocess_input", "DenseNet201", 224, 224, ""],
    "nasnetmobile": ["nasnet", "preprocess_input", "NASNetMobile", 224, 224, ""],
    "nasnetlarge": ["nasnet", "preprocess_input", "NASNetLarge", 331, 331, ""],
    "efficientnetb0": ["efficientnet", "preprocess_input", "EfficientNetB0", 224, 224, ""],
    "efficientnetb1": ["efficientnet", "preprocess_input", "EfficientNetB1", 240, 240, ""],
    "efficientnetb2": ["efficientnet", "preprocess_input", "EfficientNetB2", 260, 260, ""],
    "efficientnetb3": ["efficientnet", "preprocess_input", "EfficientNetB3", 300, 300, ""],
    "efficientnetb4": ["efficientnet", "preprocess_input", "EfficientNetB4", 380, 380, ""],
    "efficientnetb5": ["efficientnet", "preprocess_input", "EfficientNetB5", 456, 456, ""],
    "efficientnetb6": ["efficientnet", "preprocess_input", "EfficientNetB6", 528, 528, ""],
    "efficientnetb7": ["efficientnet", "preprocess_input", "EfficientNetB7", 600, 600, ""],
}
# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Yangyunchong All rights reserved.
# --------------------------------------------------------------------------------
# 2021.09.10.
# 摄影美学，用来给图片自动评分。
# 运行容器环境cuda11
#
# ================================================================================
#
import numpy as np
import torchvision.models as models
import torchvision.transforms as transforms
from Assessment.assessment.model.model import *
from components.neural.third_party.pillow import *


seed = 42
torch.manual_seed(seed)


def _predict(module,test_transform,image_info,device):
    score, variance = 0.0, 0.0
    im = pil_open(image_info)
    imt = test_transform(im)
    imt = imt.unsqueeze(dim=0)
    imt = imt.to(device)
    with torch.no_grad():
        out = module(imt)
    out = out.view(10, 1)
    for j, e in enumerate(out, 1):
        score += j * e
    for k, e in enumerate(out, 1):
        variance += e * (k - score) ** 2
    variance = variance ** 0.5
    return np.array(out.cpu()),score.item(),variance.item()


# =====================================================================


class predict():
    deeplab_model = None

    def __init__(self, model_path=None):
        if model_path == None:
            model_path = '/project/model/pretrained_model/NIMA/epoch-45.pth'
        vgg_model_path = '/project/model/pretrained_model/VGG/vgg16-397923af.pth'

        base_model = models.vgg16()
        base_model.load_state_dict(torch.load(vgg_model_path))
        self.model = NIMA(base_model)
        self.model.load_state_dict(torch.load(model_path))

        self.device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
        self.model = self.model.to(self.device)
        self.model.eval()

        self.test_transform = transforms.Compose([
            transforms.Scale(256),
            transforms.RandomCrop(224),
            transforms.ToTensor(),
            transforms.Normalize(mean=[0.485, 0.456, 0.406],
                                 std=[0.229, 0.224, 0.225])
        ])

    def predict(self, image_info):
        out, score, variance = _predict(self.model, self.test_transform, image_info, self.device)
        #
        # score = 1 -- 10
        #
        return score
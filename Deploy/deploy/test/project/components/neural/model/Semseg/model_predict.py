# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Yangyunchong All rights reserved.
# --------------------------------------------------------------------------------
# 2021.09.1.
# MIT发布的图像语义分割模型。
# 运行容器环境cuda11
#
# ================================================================================
#
import os
import numpy as np
import torch
import torch.nn as nn
import csv
from mit_semseg.dataset import TestDataset
from mit_semseg.models import ModelBuilder, SegmentationModule
from mit_semseg.utils import colorEncode, find_recursive, setup_logger
from mit_semseg.lib.nn import user_scattered_collate, async_copy_to
from mit_semseg.lib.utils import as_numpy
from PIL import Image
from mit_semseg.config import cfg 


names = {}
with open(os.path.dirname(os.path.realpath(__file__))+'/Mit_semseg/data/object150_info.csv') as f:
    reader = csv.reader(f)
    next(reader)
    for row in reader:
        names[int(row[0])] = row[5].split(";")[0]

def change_result( pred):

    pred = np.int32(pred)
    pixs = pred.size
    uniques, counts = np.unique(pred, return_counts=True)
    feature_array = np.zeros((150), dtype=float, order='C')

    for idx in np.argsort(counts)[::-1]:
        index = uniques[idx]
        ratio = counts[idx] / pixs
        if ratio > 0.001:
            feature_array[index] = ratio

    return feature_array.tolist()


def test(segmentation_module, loader, gpu,max_size= 1024):
    for batch_data in loader:
        batch_data = batch_data[0]

        segSize = (batch_data['img_ori'].shape[0],
                   batch_data['img_ori'].shape[1])
        img = batch_data['img_data'][1]
        if max(segSize) > max_size:
            k = max_size/max(img.shape)
            segSize = (int(img.shape[2]*k),int(img.shape[3]*k))
        del batch_data['img_ori']
        with torch.no_grad():
            batch_data['img_data'] = img
            batch_data = async_copy_to(batch_data, gpu)
            scores = segmentation_module(batch_data, segSize=segSize)
            _, pred = torch.max(scores, dim=1)
            pred = as_numpy(pred.squeeze(0).cpu())
        feature_array = change_result(pred)
    return feature_array


def _feature(segmentation_module,gpu,max_size):
    dataset_test = TestDataset(
        cfg.list_test,
        cfg.DATASET)
    loader_test = torch.utils.data.DataLoader(
        dataset_test,
        batch_size=cfg.TEST.batch_size,
        shuffle=False,
        collate_fn=user_scattered_collate,
        num_workers=0,
        drop_last=True)
    sem_map = test(segmentation_module, loader_test, gpu,max_size)
    return sem_map


# =====================================================================


class predict():

    def __init__(self, model_path=None):
        self.cfg_path = os.path.dirname(os.path.realpath(__file__))+'/Mit_semseg/config/ade20k-hrnetv2.yaml'   
        self.mode_path ='/project/model/pretrained_model/ade20k-hrnetv2-c1'
        self.test_checkpoint = 'epoch_30.pth'
        self.make_cfg()
        self.gup_id = 0
        torch.cuda.set_device(self.gup_id)
        self.num_classes = 151
        self.max_size = 1024
        # Network Builders
        net_encoder = ModelBuilder.build_encoder(
            arch=cfg.MODEL.arch_encoder,
            fc_dim=cfg.MODEL.fc_dim,
            weights=cfg.MODEL.weights_encoder)
        net_decoder = ModelBuilder.build_decoder(
            arch=cfg.MODEL.arch_decoder,
            fc_dim=cfg.MODEL.fc_dim,
            num_class=cfg.DATASET.num_class,
            weights=cfg.MODEL.weights_decoder,
            use_softmax=True)

        crit = nn.NLLLoss(ignore_index=-1)

        self.segmentation_module = SegmentationModule(net_encoder, net_decoder, crit)
        self.segmentation_module.cuda()
        self.segmentation_module.eval()

    def make_cfg(self):
        cfg.merge_from_file(self.cfg_path)
        cfg.MODEL.weights_encoder = os.path.join(
            self.mode_path, 'encoder_' + self.test_checkpoint)
        cfg.MODEL.weights_decoder = os.path.join(
            self.mode_path, 'decoder_' + self.test_checkpoint)

    def feature(self, image_info):
        cfg.list_test = [{'img_info': x} for x in [image_info]]
        return _feature(self.segmentation_module,self.gup_id,self.max_size)
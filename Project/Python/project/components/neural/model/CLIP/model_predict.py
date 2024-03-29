# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Chengyuanquan All rights reserved.
# --------------------------------------------------------------------------------
# 2021.08.12.
# 用来给图片按用户给定的标签自动分类。
# Clip模型代码来源于以下链接：
# copy from https://github.com/openai/CLIP
#
# 运行容器环境cuda11
#
# ================================================================================
#
import io
import numpy as np
import torch
import clip
from components.neural.third_party.pillow import *


def _load_model(model_file, gpu_ids):
    clip.available_models()
    model, preprocess = clip.load(model_file)
    if torch.cuda.is_available():
        model.cuda("cuda:"+str(gpu_ids)).eval()
    return model, preprocess


def _load_picture(picture, preprocess):
    original_images = []
    images = []
    image = pil_open(picture)
    original_images.append(image)
    images.append(preprocess(image))
    return images


def _feature_normalization(images, model, gpu_ids):
    if torch.cuda.is_available():
        image_input = torch.tensor(np.stack(images)).cuda("cuda:"+str(gpu_ids))
    else:
        image_input = torch.tensor(np.stack(images))
    with torch.no_grad():
        image_features = model.encode_image(image_input).float()
    image_features /= image_features.norm(dim=-1, keepdim=True)
    return image_features


def _load_label(ccll,gpu_ids):
    text_descriptions = [f"This is a photo of a {label}" for label in ccll]
    if torch.cuda.is_available():
        text_tokens = clip.tokenize(text_descriptions).cuda("cuda:"+str(gpu_ids))
    else:
        text_tokens = clip.tokenize(text_descriptions)
    return text_tokens, ccll


def _model_predict(image_features, model, text_tokens, ccll, top):
    with torch.no_grad():
        text_features = model.encode_text(text_tokens).float()
        text_features /= text_features.norm(dim=-1, keepdim=True)

    text_probs = (100.0 * image_features @ text_features.T).softmax(dim=-1)
    top_probs, top_labels = text_probs.cpu().topk(top, dim=-1)

    return [ccll[index] for index in top_labels[0].numpy()], top_probs[0].numpy()


def _feature_model(ccll,out_label,out_score):
    feature_array = np.zeros((len(ccll)), dtype=float, order='C')
    for index,_label in enumerate(out_label):
        if _label in ccll:
            feature_array[ccll.index(_label)] = out_score[index]

    return feature_array.tolist()


# =====================================================================


class predict():
    def __init__(self, label = None,model_file = None, gpu_ids = 0):

        if model_file == None:
            model_file = "/project/model/pretrained_model/Clip/ViT-B-32.pt"
        self.model, self.preprocess = _load_model(model_file, gpu_ids)
        self.gpu_ids = gpu_ids
        self.text_tokens, self.ccll = self.load_label(label)

    def load_label(self,label = None):
        if label == None:
            label = ['human', 'fish', 'under water', 'animal', 'one', 'single', 'who', 'is', 'a', 'I', 'how',
                     'bird', 'apple', 'aquarium_fish', 'baby', 'bear', 'beaver', 'bed', 'bee', 'beetle',
                     'bicycle', 'bottle', 'bowl', 'boy', 'bridge', 'bus', 'butterfly', 'camel', 'can', 'castle',
                     'caterpillar', 'cattle', 'chair', 'chimpanzee', 'clock', 'cloud', 'cockroach', 'couch', 'crab',
                     'crocodile', 'cup', 'dinosaur', 'dolphin', 'elephant', 'flatfish', 'forest', 'fox', 'girl',
                     'hamster', 'house', 'kangaroo', 'keyboard', 'lamp', 'lawn_mower', 'leopard', 'lion', 'lizard',
                     'lobster', 'man', 'maple_tree', 'motorcycle', 'mountain', 'mouse', 'mushroom', 'oak_tree',
                     'orange','shark','Mountains','squares', 'buildings', 'gardens', 'market',
                     'orchid', 'otter', 'palm_tree', 'pear', 'pickup_truck', 'pine_tree', 'plain', 'plate', 'poppy',
                     'porcupine', 'possum', 'rabbit', 'raccoon', 'ray', 'road', 'rocket', 'rose', 'sea', 'seal',
                     'shrew', 'skunk', 'skyscraper', 'snail', 'snake', 'spider', 'squirrel', 'streetcar', 'sunflower',
                     'sweet_pepper', 'table', 'tank', 'telephone', 'television', 'tiger', 'tractor', 'train', 'trout',
                     'tulip', 'turtle', 'wardrobe', 'whale', 'willow_tree', 'wolf', 'woman', 'worm', 'hand', 'food',
                     'valleys', 'lava', 'karst', 'glaciers', 'volcanoes', 'islands', 'rivers',
                     'lakes', 'ponds', 'waterfalls', 'springs', 'plants', 'forests', 'grasslands',
                     'ancient trees', 'famous_trees', 'flowers', 'sunrise', 'snow', 'light',
                     'mirages', 'ruins', 'ancient_tombs', 'buildings', 'gardens', 'grottoes', 'stone_carvings', 'halls',
                     'theatres', 'museums', 'folk houses', 'music', 'dance', 'murals', 'sculptures', 'streets']
        return _load_label(label,self.gpu_ids)

    def predict(self, image_info=None, top=3, text_tokens = None, ccll = None):
        self.images = _load_picture(image_info, self.preprocess)
        self.image_features = _feature_normalization(self.images, self.model,self.gpu_ids)
        if text_tokens == None:
            text_tokens = self.text_tokens
            ccll = self.ccll
        return _model_predict(self.image_features, self.model, text_tokens, ccll, top)

    def feature(self, image_info=None, top=5, text_tokens = None, ccll = None):
        out_label, out_score = self.predict(image_info,top, text_tokens, ccll )
        return _feature_model(self.ccll,out_label,out_score)
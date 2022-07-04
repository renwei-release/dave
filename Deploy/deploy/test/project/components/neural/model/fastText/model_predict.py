# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2022 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2022.01.12.
# 源代码来自：
# https://github.com/facebookresearch/fastText
# https://fasttext.cc/docs/en/python-module.html
#
# 运行容器环境cuda11
#
# ================================================================================
#
import fasttext
from components.neural.neural_tools.text.stopwords.stop_words import get_stopwords
from ltp import LTP


def _init_param(supervised_model_path):
    if supervised_model_path == None:
        supervised_model_path = \
'/project/model/trained_model/cuda11-renwei-docker/facebook_fasttext_supervised_wuyouxing_customer_service_FAQ_train_txt_20220217100639/facebook_fasttext_supervised.bin'
    return supervised_model_path


# =====================================================================


class predict():

    #
    # unsupervised_model_path 无监督学习模型，是用来计算词向量的。
    # supervised_model_path 有监督学习（学习数据集需要有分类标签），是用来给句子分类的。
    #
    def __init__(self, supervised_model_path=None):
        supervised_model_path = _init_param(supervised_model_path)
        self.supervised_model = fasttext.load_model(supervised_model_path)
        self.ltp = LTP(path = "base")
        self.stopwords = get_stopwords()
        return

    def sentence2classification(self, text=None, top=3):
        words, _ = self.ltp.seg([text])
        string_word = ''
        for word in words[0]:
            if word not in self.stopwords:
                string_word += word + ' '
        return self.supervised_model.predict(string_word, k=top)
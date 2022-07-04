# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2022 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2022.01.05.
# Gensim is commercially supported by the company rare-technologies.com, 
# who also provide student mentorships and academic thesis projects for 
# Gensim via their Student Incubator programme.
# 源代码来自：
# https://radimrehurek.com/gensim/
# https://github.com/RaRe-Technologies/gensim
#
# 运行容器环境cuda11
#
# ================================================================================
#
from gensim.models.keyedvectors import KeyedVectors
from components.neural.neural_tools.text.stopwords.stop_words import get_stopwords
from ltp import LTP


def _build_segment(content_line, ltp):
    words, _ = ltp.seg([content_line])
    return words[0]


def _init_param(word2vec_model_path):
    if word2vec_model_path == None:
        word2vec_model_path = \
'/project/model/trained_model/cuda11-renwei-docker/gensim_word2vec_zhwiki-20210901-pages-articles-multistream_xml_txt_20220113094909/gensim_word2vec.bin'
    return word2vec_model_path


# =====================================================================


class predict():

    def __init__(self, word2vec_model_path=None):
        word2vec_model_path = \
            _init_param(word2vec_model_path)

        self.word2vec_model = KeyedVectors.load_word2vec_format(word2vec_model_path, binary=True)
        self.ltp = LTP(path = "base")
        self.stopwords = get_stopwords()
        return

    def word2vec(self, text):
        words = _build_segment(text, self.ltp)
        result = {}
        for word in words:
            if word not in self.stopwords:
                similar_data = self.word2vec_model.most_similar([word])
                result[word] = similar_data
        return result
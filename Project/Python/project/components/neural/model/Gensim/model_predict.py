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
from components.neural.model.Gensim.model_train import word_cut
from ltp import LTP
from gensim.models import LdaModel

def _build_segment(content_line, ltp):
    words,_ = ltp.pipeline([content_line], tasks=["cws", "pos"]).to_tuple()
    return words[0]


def _init_param(model,model_path):
    if model_path == None:
        if model == 'word2vec':
            model_path = \
'/project/model/trained_model/AI-Test-GNC-GPU-1-cuda11-renwei-docker/gensim_word2vec_zhwiki-20210901-pages-articles-multistream_xml_txt_20220919100001/gensim_word2vec.bin'
        else:
            model_path = \
'/project/model/trained_model/AI-Test-GNC-GPU-1-cuda10-yangyunchong-docker/gensim_lda_comment_csv_20220929103207/gensim_lda.bin'
    return model_path

def _init_model(model_name,model_path):
    if model_name == 'word2vec':
        model = KeyedVectors.load_word2vec_format(model_path, binary=True)
    else:
        model = LdaModel.load(model_path)
    return model
# =====================================================================


class predict():

    def __init__(self,model_name, model_path=None):
        model_path = \
            _init_param(model_name,model_path)
        if model_name == 'word2vec':
            self.model = KeyedVectors.load_word2vec_format(model_path, binary=True)
            self.ltp = LTP()
            self.stopwords = get_stopwords()
        else:
            self.model = LdaModel.load(model_path)
        return

    def word2vec(self, text):
        words = _build_segment(text, self.ltp)
        result = {}
        for word in words:
            if word not in self.stopwords:
                similar_data = self.model.most_similar([word])
                result[word] = similar_data
        return result

    def lda(self,text):
        test_doc = word_cut(text).split(' ')
        doc_bow = self.model.id2word.doc2bow(test_doc)
        result = {}
        for i in self.model[doc_bow]:
            result[str(i[0])] = float(i[1])
        return result
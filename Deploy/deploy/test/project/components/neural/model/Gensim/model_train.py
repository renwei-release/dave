# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.12.17.
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
import sys
from public.tools import *
from gensim.models import word2vec


def _word2vec_train(train_file):
    sentences = word2vec.LineSentence(train_file)
    model = word2vec.Word2Vec(sentences=sentences)
    model_name = 'gensim_word2vec'
    model_path = t_train_path_to_model_path(train_file, model_name, None)
    model_file = model_path + '/' + model_name + '.bin'
    model.wv.save_word2vec_format(model_file, binary=True)
    return model_file


def _init_param(train_file):
    if train_file == None:
        #
        # /project/dataset/Public/dumps.wikimedia.org/zhwiki-20210901-pages-articles-multistream.xml.txt
        #      the file come from /project/dave/neural_network/neural_tools/text/wiki.py
        # '/project/dataset/Public/dumps.wikimedia.org/wiki_00'
        #
        train_file = '/project/dataset/Public/dumps.wikimedia.org/zhwiki-20210901-pages-articles-multistream.xml.txt'
    return train_file


def _model_train(train_file):
    model_path = _word2vec_train(train_file)
    return model_path


# =====================================================================


def train(train_file = None):
    train_file = _init_param(train_file)
    model_path = _model_train(train_file)
    print(f'model_path:{model_path}')
    return model_path


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        train(train_file=sys.argv[1])
    else:
        train()
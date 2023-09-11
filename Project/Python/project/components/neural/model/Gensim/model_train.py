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
import re
import pandas as pd
from public.tools import *
from gensim.models import word2vec
from gensim.models import LdaModel
from gensim.corpora import Dictionary
from gensim.test.utils import datapath
import jieba
import jieba.posseg as psg
import nltk

def word_cut(mytext):
    jieba.initialize()  # 手动初始化（可选）

    # 加载用户停用词表
    stop_list = []  # 存储用户停用词
    flag_list = ['n', 'nz', 'vn']  # 指定在jieba.posseg分词函数中只保存n：名词、nz：其他专名、vn：动名词

    word_list = []
    seg_list = psg.cut(mytext)  # jieba.posseg分词
    # 原称之为粑粑山型的 词语过滤
    for seg_word in seg_list:
        word = re.sub(u'[^\u4e00-\u9fa5]', '', seg_word.word)  # 只匹配所有中文
        find = 0  # 标志位
        for stop_word in stop_list:
            if stop_word == word or len(word) < 2:  # 长度小于2或者在用户停用词表中，将被过滤
                find = 1
                break
        if find == 0 and seg_word.flag in flag_list:  # 标志位为0且是需要的词性则添加至word_list
            word_list.append(word)

    tokens = nltk.word_tokenize(mytext)
    pos_tags = nltk.pos_tag(tokens)
    for word, pos in pos_tags:
        word = ''.join(re.findall(r'[A-Za-z]', word))
        if len(word) < 2 :
            continue
        if (pos == 'NN' or pos == 'NNP' or pos == 'NNS' or pos == 'NNPS'):
            word_list.append(word)
    return (" ").join(word_list)

def _word2vec_train(train_file):
    sentences = word2vec.LineSentence(train_file)
    model = word2vec.Word2Vec(sentences=sentences)
    model_name = 'gensim_word2vec'
    model_path = t_train_path_to_model_path(train_file, model_name, None)
    model_file = model_path + '/' + model_name + '.bin'
    model.wv.save_word2vec_format(model_file, binary=True)
    return model_file

def _lda_train(train_file,column='detail'):
    data = pd.read_csv(train_file)
    data["content_cutted"] = data[column].loc[0:1000].apply(word_cut)
    data_list = []
    for i in data["content_cutted"]:
        if isinstance (i,str):
            data_list.append(i.split(' '))
    dictionary = Dictionary(data_list)
    dictionary.filter_n_most_frequent(200)
    corpus = [dictionary.doc2bow(text) for text in data_list]
    lda = LdaModel(corpus=corpus, id2word=dictionary, num_topics=10)
    model_name = 'gensim_lda'
    model_path = t_train_path_to_model_path(train_file, model_name, None)
    model_file = model_path + '/' + model_name + '.bin'
    lda.save(datapath(model_file))
    return model_file

def _init_param(train_file,model):
    if train_file == None:
        #
        # /project/dataset/Public/dumps.wikimedia.org/zhwiki-20210901-pages-articles-multistream.xml.txt
        #      the file come from /project/dave/neural_network/neural_tools/text/wiki.py
        # '/project/dataset/Public/dumps.wikimedia.org/wiki_00'
        #
        if model == 'word2vec':
            train_file = '/project/dataset/Public/dumps.wikimedia.org/zhwiki-20210901-pages-articles-multistream.xml.txt'
        elif  model == 'lda':
            train_file = '/project/dataset/Public/recommendation/original_data_tripadvisor/csv/france/comment.csv'

    return train_file


def _model_train(train_file,model):
    if model == 'word2vec':
        model_path = _word2vec_train(train_file)
    else:
        model_path = _lda_train(train_file)
    return model_path


# =====================================================================


def train(train_file = None,model = 'lda'):  # 参数model  为word2vec或lda
    train_file = _init_param(train_file,model)
    model_path = _model_train(train_file,model)
    print(f'model_path:{model_path}')
    return model_path


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        train(train_file=sys.argv[1])
    else:
        train()
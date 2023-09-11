# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
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
import sys
import fasttext
import os
from public.tools import *


def _unsupervised_train(train_file):
    sub_model = 'cbow' # 'cbow' 'skipgram'
    model_name = 'facebook_fasttext_unsupervised'
    model_path = t_train_path_to_model_path(train_path=train_file, model_name=model_name, misc_str=sub_model)
    model_file = model_path + '/' + model_name + '.bin'

    model = fasttext.train_unsupervised(input=train_file, model=sub_model)
    model.save_model(model_file)
    return model_file


def _supervised_train(train_file, valid_file):
    model_name = 'facebook_fasttext_supervised'
    model_path = t_train_path_to_model_path(train_path=train_file, model_name=model_name)

    if valid_file != None:
        #
        # 带验证集训练时，目前Python版本会有Core Dump事件，故此处直接用系统命令训练
        #
        cmd_file = model_path + '/' + model_name
        model_file = cmd_file + '.bin'
        os.system(f'fasttext supervised -input {train_file} -autotune-validation {valid_file} -output {cmd_file} -lr 0.1 -dim 200 -wordNgrams 2 -epoch 500 -neg 5 -label \'__label__\'')
    else:
        model = fasttext.train_supervised(input=train_file, lr=0.1, dim=200, wordNgrams=2, epoch=500, neg=5, label='__label__')
        model_file = model_path + '/' + model_name + '.bin'
        model.save_model(model_file)
    return model_file


def _get_model_name(train_file):
    find_table = False
    with open(train_file) as f:
        if '__label__' in f.read():
            find_table = True
    if find_table == True:
        return 'supervised'
    else:
        return 'unsupervised'


def _init_param(train_file, valid_file):
    if train_file == None:
        #
        # train_file = /project/dataset/Public/dumps.wikimedia.org/zhwiki-20210901-pages-articles-multistream.xml.txt
        #      the file come from /project/dave/neural_network/neural_tools/text/wiki.py
        # train_file = '/project/dataset/Public/dumps.wikimedia.org/wiki_00'
    
        # train_file = '/project/dataset/Public/YFCC100M/YFCC100M/train'
        # train_file = '/project/dataset/Public/YFCC100M/YFCC100M/valid'

        # train_file = '/project/dataset/Public/stackexchange/cooking.stackexchange.txt'

        # train_file = '/project/dataset/Private/wuyouxing_customer_service_FAQ/wuyouxing_customer_service_FAQ.txt'
        train_file = '/project/dataset/Private/wuyouxing_customer_service_FAQ/wuyouxing_customer_service_FAQ_train.txt'
        valid_file = '/project/dataset/Private/wuyouxing_customer_service_FAQ/wuyouxing_customer_service_FAQ_valid.txt'
    model_name = _get_model_name(train_file)
    return model_name, train_file, valid_file


def _model_train(model_name, train_file, valid_file):
    if model_name == 'unsupervised':
        model_path = _unsupervised_train(train_file)
    elif model_name == 'supervised':
        model_path = _supervised_train(train_file, valid_file)
    else:
        model_path = _unsupervised_train(train_file)
    return model_path


# =====================================================================


def train(train_file = None, valid_file = None):
    model_name, train_file, valid_file = _init_param(train_file, valid_file)
    model_path = _model_train(model_name, train_file, valid_file)
    print(f'model_name:{model_name} model_path:{model_path}')
    return model_path


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        train(train_file=sys.argv[1])
    else:
        train()
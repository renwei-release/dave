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
import json
import sys
from components.neural.model.fastText.model_predict import predict


def _test_FAQ_benchmarking():
    test_text_array = [\
'我要拨打黄页电话',
'电话出故障啦',
'境内特服号码是不是换了啊',
'如何开启号码托管',
'无忧行的流量包有效使用期吗？',
'流量使用完后超出部分计算费用吗',
'如何使用亲情号码',
'取消号码托管',
'我的流量包无法正常使用',
'如何进行设备安全验证',
'我当天使用话费扣到30封顶后又购买了两天的流量包，那么这时候流量是如何使用的']

    print(f'benchmarking test ...')

    fasttext_predict = predict()
    for test_text in test_text_array:
        classification_data = fasttext_predict.sentence2classification(text=test_text, top=1)
        label = classification_data[0]
        score = classification_data[1]
        print(f'{test_text} -> {label}/{score}')
    return


def _test_FAQ_talk():
    faq_table_file = '/project/dataset/Private/wuyouxing_customer_service_FAQ/wuyouxing_customer_service_QA.json'

    fasttext_predict = predict()
    load_dict = {}
    with open(faq_table_file, 'r', encoding='UTF-8') as f:
        load_dict = json.load(f)
    talk_flag = True

    print('我们开始对话吧！（输入exit退出）')

    while talk_flag == True:
        question = input()
        if question == 'exit':
            print('再见！')
            break

        classification_data = fasttext_predict.sentence2classification(text=question, top=1)
        label = str(classification_data[0][0]).replace('__label__', '')
        score = float(classification_data[1][0])
        answer = load_dict.get(label, '不好意思，我不知道你在说什么！')

        print(f'\033[95m->\033[0m{answer}\n')
    return


# =====================================================================


def test():
    _test_FAQ_benchmarking()
    _test_FAQ_talk()
    return


if __name__ == "__main__":
    test()
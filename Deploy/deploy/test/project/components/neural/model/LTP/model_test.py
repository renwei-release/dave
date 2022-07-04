# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.12.14.
# LTP提供了一系列中文自然语言处理工具，
# 用户可以使用这些工具对于中文文本进行分词、
# 词性标注、句法分析等等工作。
# 源代码来自：
# https://github.com/HIT-SCIR/ltp
# http://static.scir.yunfutech.com/source/ltp-3.4.0-SourceCode.zip
# 运行容器环境cuda11
#
# ================================================================================
#
import json
from components.neural.model.LTP.model_predict import predict as ltp_predict


ltp_test_sentences = [
    "你是军人吗？我觉得我是一个黑人。我叫迈克杰克逊。",
    "示例程序通过命令行参数指定模型文件路径。第11行加载模型文件，并将命名实体识别器指针存储在engine中。",
    "第21至30行构造分词序列words和词性标注序列postags，第34行运行词性标注逻辑，并将结果存储在名为tags的std::vector<std::string>中。第40行释放分词模型。",
    "随着对业务理解的不断深入和抽象，可以发现很多业务场景的功能(代码)都可以抽象成“规则+指标”的模式。这种模式，可以应用于很多场景",
    "你喜欢吃苹果吗？苹果什么味道。苹果苹果橘子句子桌子桌子我要去成都四川重庆"
]

# =====================================================================


def test():
    ltp = ltp_predict()
    for ltp_test_sentence in ltp_test_sentences:
        json_data = json.dumps(ltp.predict(ltp_test_sentence), ensure_ascii=False, indent=4)    
        print(f'{json_data}')
    return


if __name__ == "__main__":
    test()
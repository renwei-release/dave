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
import torch
from ltp import LTP


# =====================================================================


class predict():

    def __init__(self, user_dict=None):
        self.ltp = LTP("LTP/Base")   # "base|small|tiny"
        if user_dict != None:
            self.ltp.init_dict(path=user_dict, max_window=4)
        if torch.cuda.is_available():
            self.ltp.to("cuda")
        return

    def predict(self, text):
        output = self.ltp.pipeline([text], tasks=["cws", "pos", "ner", "srl", "dep", "sdp"])

        total_result = []
        total_result.append([ {'cws': output.cws}, {'pos': output.pos}, {'ner': output.ner}, {'srl': output.srl}, {'dep': output.dep}, {'sdp': output.sdp} ])
        return total_result
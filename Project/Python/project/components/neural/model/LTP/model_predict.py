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

    def predict(self, text, tasks=None):
        if tasks == None:
            tasks = ["cws", "pos", "ner", "srl", "dep", "sdp"]

        if 'cws' not in tasks:
            tasks.append('cws')

        output = self.ltp.pipeline([text], tasks=tasks)

        total_result = {}

        for task in tasks:
            if task == "cws":
                total_result[task] = output.cws[0]
            elif task == "pos":
                total_result[task] = output.pos[0]
            elif task == "ner":
                total_result[task] = output.ner[0]
            elif task == "srl":
                total_result[task] = output.srl[0]
            elif task == "dep":
                total_result[task] = output.dep[0]
            elif task == "sdp":
                total_result[task] = output.sdp[0]

        return total_result
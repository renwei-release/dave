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
from ltp import LTP


# =====================================================================


class predict():

    def __init__(self, user_dict=None):
        self.ltp = LTP(path = "base")   # "base|small|tiny"
        if user_dict != None:
            self.ltp.init_dict(path=user_dict, max_window=4)
        return

    def predict(self, text):
        total_result = []
        # 分句
        sentences = self.ltp.sent_split([text])
        for sentence in sentences:
            # 分词
            seg, hidden = self.ltp.seg([sentence])
            # 词性标注
            pos = self.ltp.pos(hidden)
            # 命名实体识别
            ner = self.ltp.ner(hidden)
            # 语义角色标注
            srl = self.ltp.srl(hidden)
            # 依存句法分析
            dep = self.ltp.dep(hidden)
            # 语义依存分析(树)
            sdp = self.ltp.sdp(hidden, mode='tree')

            total_result.append([ {'sentence':sentence}, {'seg' : seg}, {'pos':pos}, {'ner':ner}, {'srl':srl}, {'dep':dep}, {'sdp':sdp} ])
        return total_result
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
    "人工客服谢谢",
    '需要人工客服',
    '转人工',
    '多谢',
]

# =====================================================================


def test():
    ltp = ltp_predict()
    for ltp_test_sentence in ltp_test_sentences:
        json_data = json.dumps(ltp.predict(ltp_test_sentence, ["srl"]), ensure_ascii=False, indent=4)    
        print(f'{json_data}')
    return


if __name__ == "__main__":
    test()
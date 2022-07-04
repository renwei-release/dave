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
import json
from components.neural.model.Gensim.model_predict import predict


def _test_word2vec():
    gensim_predict = predict()
    similar_word = gensim_predict.word2vec('训练已经完成，我们得到了想要的模型以及词向量，并保存到本地。')
    json_str = json.dumps(similar_word, ensure_ascii=False, indent=2)
    print(f'similar_word:{json_str}')
    return


# =====================================================================


def test():
    _test_word2vec()
    return


if __name__ == "__main__":
    test()
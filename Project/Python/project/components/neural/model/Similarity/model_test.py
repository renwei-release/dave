# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2023 Renwei All rights reserved.
# --------------------------------------------------------------------------------
#
# 运行容器环境cuda11
#
# ================================================================================
import time


similarity_texts = [
    "我是北京人",
    "我是湖南人",
    "今天吃炸酱面",
    "你希望我打电话过来吗",
    "CloudSMS如何申报故障？",
    "你吃饭了吗？想不想吃杂酱面？",
    "我发现了一个故障，应该如何申报？",
    "我要购买流量包",
    "我要购买上网套餐",
    "美联储加息引起了市场的恐慌",
    "美联储减息引起了市场的利好",
    "客服电话",
    "无忧行客服电话",
    "无忧行客服 联系方式 收费",
    "拨打客服电话是否收费",
    "在无忧行APP拨打或接听电话产生异常收费问题",
    "通过无忧行APP拨打电话，需要付费和消耗流量吗",
    "无忧行的官网是什么?"
]


def _similarity_predict_topn(model_name, model_predict):
    texts_len = len(similarity_texts)
    if texts_len < 2:
        return

    print(f"=============={model_name}==============")
    for texts_1_index, text_1 in enumerate(similarity_texts):
        if texts_1_index >= texts_len - 1:
            break
        topN_result = {}
        for texts_2_index, text_2 in enumerate(similarity_texts):
            if texts_2_index != texts_1_index:
                similarity_result = model_predict.predict(text_1, text_2)
                if similarity_result > 0.3:
                    topN_result[text_2] = similarity_result
        topN_result = sorted(topN_result.items(), key=lambda x: x[1], reverse=True)
        print(f"{text_1} / {topN_result}\n")
    return


def _similarity_predict_total_test():
    from components.neural.model.Similarity.bge_text2Vec.model_predict import predict as Text2Vec_predict
    _similarity_predict_topn("Text2vec", Text2Vec_predict(None))

    #from components.neural.model.Similarity.TfidfVectorizer.model_predict import predict as TfidfVectorizer_predict
    #_similarity_predict_topn("TfidfVectorizer", TfidfVectorizer_predict())

    #from components.neural.model.Similarity.CountVectorizer.model_predict import predict as CountVectorizer_predict
    #_similarity_predict_topn("CountVectorizer", CountVectorizer_predict())

    #from components.neural.model.Similarity.BERT.model_predict import predict as BERT_predict
    #_similarity_predict_topn("BERT", BERT_predict())

    #from components.neural.model.Similarity.Azure.model_predict import predict as Azure_predict
    #_similarity_predict_topn("Azure", Azure_predict())

    #from components.neural.model.Similarity.Transformers.model_predict import predict as Transformers_predict
    #_similarity_predict_topn("Transformers", Transformers_predict())

    #from components.neural.model.Similarity.Doc2Vec.model_predict import predict as Doc2Vec_predict
    #_similarity_predict_topn("Doc2Vec", Doc2Vec_predict())

    #from components.neural.model.Similarity.Word2Vec.model_predict import predict as Word2Vec_predict
    #_similarity_predict_topn("Word2Vec", Word2Vec_predict())


def _encode_test():
    from components.neural.model.Similarity.Text2Vec.model_predict import predict as Similarity_predict

    model = Similarity_predict("/project/model/trained_model/AI-Test-GNC-GPU-1-cuda11-renwei-docker/GanymedeNil/text2vec-large-chinese_JegoTrip_20230619064028")

    start = time.time()
    vector1 = model.encode("我是北京人")
    print(f"encode time: {time.time() - start}")
    start = time.time()
    vector2 = model.encode("我是湖南人")
    print(f"encode time: {time.time() - start}")
    start = time.time()
    vector3 = model.encode("吃饭")
    print(f"encode time: {time.time() - start}")

    vector1 = [ vector1 ]
    vectors = [ vector2, vector3 ]

    start = time.time()
    ret = model.cosine(vector1, vectors)
    print(f"cosine time: {time.time() - start}")

    print(ret)
    return


# =====================================================================


def test():
    _encode_test()
    return


if __name__ == "__main__":
    test()
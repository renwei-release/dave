# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2023 Renwei All rights reserved.
# --------------------------------------------------------------------------------
#
# 运行容器环境cuda11
#
# ================================================================================
from components.neural.model.Similarity.bge_text2Vec.model_predict import predict as Text2Vec_predict


# =====================================================================


class predict():

    def __init__(self, model_path=None):
        self.model = Text2Vec_predict(model_path=model_path)
        return

    def predict(self, text1, text2):
        return self.model.predict(text1, text2)

    def encode(self, text):
        return self.model.encode(text)
    
    def cosine(self, vector1, vector2):
        return self.model.cosine(vector1, vector2)
    
    def version(self):
        return self.model.version()
# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2023 CAOJINGLEI All rights reserved.
# --------------------------------------------------------------------------------
#
# 运行容器环境cuda11
#
# ================================================================================
from sentence_transformers import SentenceTransformer
from sklearn.metrics.pairwise import cosine_similarity
import os


# =====================================================================


class predict():

    def __init__(self, model_path=None, device='cpu'):
        if (model_path is None) or (model_path == "") or (os.path.exists(model_path) is False):
            model_path = 'BAAI/bge-large-zh'
        self.model_path = model_path
        self.sentence_model = SentenceTransformer(model_path, device = device)
        return

    def predict(self, text1, text2):
        corpus = [text1, text2]
        vectors = self.sentence_model.encode(corpus, normalize_embeddings=True)
        similarity = cosine_similarity(vectors)
        return similarity[0][1]
    
    def encode(self, text):
        return self.sentence_model.encode(text, normalize_embeddings=True ) 

    def cosine(self, vector1, vector2):
        return cosine_similarity(X=vector1, Y=vector2, dense_output=True).astype('float32')

    def version(self):
        return self.model_path
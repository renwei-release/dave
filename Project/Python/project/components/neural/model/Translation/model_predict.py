# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2023 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# ================================================================================
#
from transformers import AutoTokenizer, AutoModelForSeq2SeqLM, pipeline


# =====================================================================


class predict():

    def __init__(self):
        self.zh_to_en_tokenizer = AutoTokenizer.from_pretrained("Helsinki-NLP/opus-mt-zh-en")
        self.zn_to_en_model = AutoModelForSeq2SeqLM.from_pretrained("Helsinki-NLP/opus-mt-zh-en")
        self.zh_to_en_predict = pipeline("translation_zh_to_en", model=self.zn_to_en_model, tokenizer=self.zh_to_en_tokenizer)
        return

    def zh_to_en(self, zh_text=None):
        if zh_text is None:
            return None

        en_text_array = self.zh_to_en_predict(zh_text, max_length=1024)

        return en_text_array[0]['translation_text']
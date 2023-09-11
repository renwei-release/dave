# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2023 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# ================================================================================
#
from components.neural.model.Translation.model_predict import predict as translation_predict


# =====================================================================


def test():
    translation = translation_predict()

    zh_text = '这是一段中文文本'
    en_text = translation.zh_to_en(zh_text)
    print(f"{zh_text} -> {en_text}")
    return


if __name__ == "__main__":
    test()
# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 ChengYuanquan All rights reserved.
# --------------------------------------------------------------------------------
# 2021.07.28.
# EasyOCR可以对多语言进行文字识别，效果较优。
# 此红酒识别需要对红酒商标上的多国语言进行识别。
#
# Ocr的代码来自以下链接：
# https://github.com/JaidedAI/EasyOCR
#
#
# 运行容器环境cuda10,cudnn7
#
# ================================================================================
import easyocr
from paddleocr import PaddleOCR, draw_ocr


def _load_model(language):
    reader = easyocr.Reader(['ch_sim', language])  # this needs to run only once to load the model into memory
    return reader
    
def _load_model2(language):
    ocr = PaddleOCR(use_angle_cls=True, lang=language)  # need to run only once to download and load model into memory
    return ocr

def _ocr_predict_EasyOcr(image_file,reader):
    result = reader.readtext(image_file)
    result = easyresult_trans(result)
    return result

# Paddleocr目前支持的多语言语种可以通过修改lang参数进行切换。例如`ch`, `en`, `fr`, `german`, `korean`, `japan`
def _ocr_predict_PaddleOcr(image_file,ocr):
    result = ocr.ocr(image_file, cls=True)
    return result

def easyresult_trans(result):
    for i in range(len(result)):
        result[i] = list(result[i])
        for j in range(len(result[i][0])):
            result[i][0][j][0],result[i][0][j][1] = float(result[i][0][j][0]),float(result[i][0][j][1])
        result[i][1] = (result[i][1],result[i][2])
        del result[i][2]
    return result

def _compare_ocrnumber(result):
    total=0
    for i in range(len(result)):
        total=total+result[i][1][1]
    try:
        return total/len(result)
    except:
        return 0
    
def _model_choice(easy_result,padd_result):
    if easy_result >= 0.5:
        return 1
    else:
        if easy_result < padd_result:
            return 0
        else:
            return 1
            
class predict():
    def __init__(self, language=None):
        if language == None:
            language = 'en'
        self.elang=language
        self.model=_load_model(self.elang)
        self.model2=_load_model2(self.elang)


    def predict(self, image_info=None):
        if image_info == None:
            return None
        
        self.easyocr_predict = _ocr_predict_EasyOcr(image_info, self.model)
        self.paddocr_predict = _ocr_predict_PaddleOcr(image_info, self.model2)
        self.easy_result =_compare_ocrnumber(self.easyocr_predict)
        self.padd_result =_compare_ocrnumber(self.paddocr_predict)
        
        if _model_choice(self.easy_result,self.padd_result) ==1:
            return self.easyocr_predict
        else:
            return self.paddocr_predict


if __name__ == "__main__":
    outcome=predict()
    s=outcome.predict()
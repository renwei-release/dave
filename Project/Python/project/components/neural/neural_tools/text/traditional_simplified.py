#!/usr/bin/python
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.12.29.
# ================================================================================
#
import os
from public.tools import *
from zhtools.langconv import *


def _Traditional2Simplified(sentence):
    '''
    将sentence中的繁体字转为简体字
    :param sentence: 待转换的句子
    :return: 将句子中繁体字转换为简体字之后的句子
    '''
    sentence = Converter('zh-hans').convert(sentence)
    return sentence


def _Simplified2Traditional(sentence):
    '''
    将sentence中的简体字转为繁体字
    :param sentence: 待转换的句子
    :return: 将句子中简体字转换为繁体字之后的句子
    '''
    sentence = Converter('zh-hant').convert(sentence)
    return sentence


def _traditional_simplified(text_path, result_path, traditional_simplified_fun):
    file_list, file_number = t_path_file_list(file_path=text_path, suffix=None)
    progress_bar = t_print_progress_bar(file_number)

    for read_name in file_list:
        progress_bar.show()

        with open(read_name, "r", encoding="utf-8") as f_read:
            sentence = traditional_simplified_fun(f_read.read())

            write_path = result_path + '/' + read_name.split('/', 1)[-1].split('/', 1)[-1].rsplit('/', 1)[0]
            os.system(f'mkdir -p {write_path}')
            write_name = write_path + '/' + read_name.rsplit('/', 1)[-1]

            with open(write_name, "w", encoding="utf-8") as f_write:
                f_write.write(sentence)
    return


# =====================================================================


def Traditional2Simplified(input_text_path):
    output_text_path = input_text_path + '_Traditional2Simplified'
    _traditional_simplified(input_text_path, output_text_path, _Traditional2Simplified)
    return output_text_path


def Simplified2Traditional(input_text_path):
    output_text_path = input_text_path + '_Simplified2Traditional'
    _traditional_simplified(input_text_path, output_text_path, _Simplified2Traditional)
    return output_text_path


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        text_path = sys.argv[1]
    else:
        text_path = './wikiextractor_output_text'
    Traditional2Simplified(text_path)
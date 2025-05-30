# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import string
import unicodedata
import re


# =====================================================================


def t_stdio_string_split(want_split_string, split_flag):
    index = want_split_string.find(split_flag)

    has_split_flag = False

    if index != -1:
        index_after_prefix = index + len(split_flag)
        want_split_string = want_split_string[index_after_prefix:]
        has_split_flag = True

    return want_split_string, has_split_flag


def t_stdio_string_remove_punctuation(input_string):
    translator = str.maketrans('', '', string.punctuation)
    return input_string.translate(translator)


def t_stdio_decode_unicode(text):
    # 定义一个转换函数，将 /uniXXXX 转换为对应的 Unicode 字符
    def decode_match(match):
        hex_value = match.group(1)
        try:
            return chr(int(hex_value, 16))
        except Exception:
            return match.group(0)

    # 使用正则表达式匹配 /uni 后面跟4个16进制数字
    decoded_text = re.sub(r"/uni([0-9A-Fa-f]{4})", decode_match, text)
    return decoded_text


def t_stdio_fullwidth_to_halfwidth(ustring):
    rstring = ""
    for uchar in ustring:
        inside_code = ord(uchar)
        if inside_code == 0x3000:
            inside_code = 0x0020
        else:
            inside_code -= 0xfee0
        if inside_code < 0x0020 or inside_code > 0x7e:
            rstring += uchar
        else:
            rstring += chr(inside_code)
    return rstring
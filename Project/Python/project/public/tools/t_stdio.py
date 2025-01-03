# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import string
import unicodedata


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
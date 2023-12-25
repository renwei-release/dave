# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import unicodedata


# =====================================================================


def t_unicodedata_check_language(s):
    def unicode_block(c):
        return unicodedata.name(c).split()[0]

    blocks = set(map(unicode_block, s))
    if 'CJK' in blocks:
        return 'Chinese'
    elif 'LATIN' in blocks:
        return 'English'
    elif 'HIRAGANA' in blocks or 'KATAKANA' in blocks:
        return 'Japanese'
    elif 'HANGUL' in blocks:
        return 'Korean'
    elif 'CYRILLIC' in blocks:
        return 'Russian'
    elif 'ARABIC' in blocks:
        return 'Arabic'
    elif 'HEBREW' in blocks:
        return 'Hebrew'
    elif 'GREEK' in blocks:
        return 'Greek'
    else:
        return 'Other'
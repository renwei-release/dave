# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import unicodedata


unicodedata_dataset = [
    "我是北京人",
    "CloudSMS如何申报故障？",
    "APP是什么",
    "What is APP？",
    "通过无忧行APP拨打电话，需要付费和消耗流量吗",
    "无忧行的官网是什么?",
    "请间春节期间具体放假安排是怎样的，我想知道具体有多少天假期。",
    "托管天数需要达到多少天才有奖品?",
    "hello",
    "你好",
    "こんにちは",
    "안녕하세요",
    "안녕하세요你好",
    "你是誰",
    "データパッケージの購入方法",
    "你好\r\n",
    "美版 iphone 14 pro max 怎么开启号码夺冠"
]


def _t_unicodedata_check_language(block):
    if block == None:
        return None

    if 'CJK' in block:
        return 'Chinese'
    elif 'LATIN' in block:
        return 'English'
    elif ('HIRAGANA' == block) or ('KATAKANA' == block) or ('KATAKANA-HIRAGANA' == block):
        return 'Japanese'
    elif 'HANGUL' == block:
        return 'Korean'
    elif 'CYRILLIC' == block:
        return 'Russian'
    elif 'ARABIC' == block:
        return 'Arabic'
    elif 'HEBREW' == block:
        return 'Hebrew'
    elif 'GREEK' == block:
        return 'Greek'
    else:
        return None


# =====================================================================


def t_unicodedata_check_language(s):
    if (s == None) or (s == ''):
        return 'Chinese'

    def unicode_block(c):
        if c.isprintable() == False:
            return None
        if (c == None) or (len(c) == 0):
            return None
        try:
            ret = unicodedata.name(c).split()[0]
        except Exception as e:
            print(f"unicode_block error:{e} c:{c}")
            ret = None
        return ret

    language_counter = {}
    
    blocks = list(map(unicode_block, s))

    for block in blocks:
        language = _t_unicodedata_check_language(block)
        if language is None:
            continue
        if language not in language_counter:
            language_counter[language] = 1
        else:
            language_counter[language] += 1

    if len(language_counter) == 0:
        return 'Chinese'

    if ('Chinese' in language_counter) and ('English' in language_counter):
        return 'Chinese'

    return max(language_counter, key=language_counter.get)


if __name__ == "__main__":
    for text in unicodedata_dataset:
        print(f"{text} -> {t_unicodedata_check_language(text)}")
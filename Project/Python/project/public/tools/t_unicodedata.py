# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import unicodedata
import re


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
    elif 'DEVANAGARI' == block:
        return 'Hindi'
    elif 'THAI' == block:
        return 'Thai'
    elif 'MALAY' == block:
        return 'Malay'
    elif 'INDONESIAN' == block:
        return 'Indonesian'
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

def remove_garbage_characters(text):
    """
    删除 Unicode 私有区域字符和无效字符
    （范围覆盖常见乱码区：U+E000-U+F8FF, U+F0000-U+FFFFD, U+100000-U+10FFFD）
    """
    cleaned = []
    for char in text:
        code_point = ord(char)
        # 保留合法字符：常规字符 + 常用符号 + 中日韩文字
        if (
            (0x0000 <= code_point <= 0xD7FF) or    # 基本多文种平面
            (0xE000 <= code_point <= 0xFFFD) or    # 私有区域外的特殊符号
            (0x10000 <= code_point <= 0x10FFFF)    # 扩展平面（如emoji）
            and not (
                (0xE000 <= code_point <= 0xF8FF) or    # 常见私有区域
                (0xF0000 <= code_point <= 0xFFFFD) or   # 补充私有区域
                (0x100000 <= code_point <= 0x10FFFD)    # 扩展私有区域
            )
        ):
            cleaned.append(char)
        else:
            continue  # 丢弃乱码字符
    
    # 二次清理：移除连续空行和图片Base64数据
    cleaned_text = "".join(cleaned)
    cleaned_text = re.sub(r"!\[\]\(data:image.*?\)", "", cleaned_text)  # 删除图片标记
    cleaned_text = re.sub(r"\n{3,}", "\n\n", cleaned_text)  # 合并多余空行
    
    return cleaned_text.strip()


if __name__ == "__main__":
    for text in unicodedata_dataset:
        print(f"{text} -> {t_unicodedata_check_language(text)}")
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


def t_stdio_remove_the_middle_space(string):
    space = r"[ \t\u3000\u00A0]+"
    # 常见中文标点
    punct = r"，。！？；：、“”‘’（）《》【】"

    # 去掉“中文 + 空白 + 中文”的空格（不处理换行）
    string = re.sub(
        rf"([\u4e00-\u9fa5]){space}([\u4e00-\u9fa5])",
        r"\1\2",
        string
    )

    # 去掉“中文 + 空白 + 标点”的空格（不处理换行）
    string = re.sub(
        rf"([\u4e00-\u9fa5]){space}([{punct}])",
        r"\1\2",
        string
    )

    # 去掉“标点 + 空白 + 中文”的空格（不处理换行）
    string = re.sub(
    rf"([{punct}]){space}([\u4e00-\u9fa5])",
        r"\1\2",
        string
    )

    return string


def t_stdio_remove_all_punctuation(string):
    if not string:
        return ""

    # 1. 移除空白字符（空格、换行、制表符等）
    string = "".join(string.split())

    # 2. 移除所有标点符号（包括中英文标点）
    # \W 会匹配非字母数字下划线的字符（在 Unicode 模式下包括大部分标点）
    # _ 也会被作为标点移除（如果需要的话）
    string = re.sub(r'[^\w\s]', '', string, flags=re.UNICODE)
    # 如果想更彻底地去掉下划线：
    string = string.replace('_', '')

    return string


def t_stdio_remove_all_emoji(string):
    if not string:
        return ""

    # 1) 移除旗帜 Emoji（由两个区域指示符组成）
    string = re.sub(r"[\U0001F1E6-\U0001F1FF]{2}", "", string)

    # 2) 移除键帽 Emoji（如 1️⃣、#️⃣、*️⃣）
    string = re.sub(r"[0-9#*]\uFE0F?\u20E3", "", string)

    # 3) 移除主要 Emoji 区间及其组合（含肤色修饰符和 ZWJ 连接）
    supp_emoji = r"[\U0001F000-\U0001FAFF\U0001FC00-\U0001FFFD]"
    modifier = r"[\U0001F3FB-\U0001F3FF]?"
    emoji_sequence = re.compile(
        rf"{supp_emoji}{modifier}(?:\uFE0F)?(?:\u200D{supp_emoji}{modifier}(?:\uFE0F)?)*",
        flags=re.UNICODE
    )
    string = emoji_sequence.sub("", string)

    # 4) 仅移除显式 Emoji 形式的 BMP 符号（带 VS16: U+FE0F）
    string = re.sub(
        r"[\u2600-\u26FF\u2700-\u27BF\u2300-\u23FF\u2194-\u21FF\u2934-\u2935\u2B05-\u2B55]\uFE0F",
        "",
        string,
        flags=re.UNICODE
    )

    return string


def t_stdio_sql_escape(input_string):
    if input_string is None:
        return None
    # 替换单引号为两个单引号
    escaped_string = input_string.replace("'", "''")
    # 替换反斜杠为两个反斜杠
    escaped_string = escaped_string.replace("\\", "\\\\")
    return escaped_string
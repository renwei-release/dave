#!/usr/bin/python
#
# ================================================================================
# (c) Copyright 2021-2022 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.12.29.
# 2022.01.01.
# ================================================================================
#
import os
import sys
import re
from traditional_simplified import Traditional2Simplified
from public.tools import *
from components.neural.neural_tools.text.stopwords.stop_words import get_stopwords
from ltp import LTP


def _build_segment(content_line, ltp):
    words, _ = ltp.seg([content_line])
    return words[0]


def _parse_zhwiki_words(read_file, save_file, ltp):
    file = open(read_file, "r", encoding="utf-8")
    #写文件
    output = open(save_file, "w+", encoding="utf-8")
    content_line = file.readline()
    #获取停用词表
    stopwords = get_stopwords()
    #定义一个字符串变量，表示一篇文章的分词结果
    article_contents = ""
    #过滤掉<doc>
    regex_str = "[^<doc.*>$]|[^</doc>$]"
    while content_line:
        match_obj = re.match(regex_str, content_line)
        content_line = content_line.strip("\n")
        if len(content_line) > 0:
            if match_obj:
                words = _build_segment(content_line, ltp)
                for word in words:
                    if word not in stopwords:
                        article_contents += word + " "
            else:
                if len(article_contents) > 0:
                    output.write(article_contents + "\n")
                    article_contents = ""
        content_line = file.readline()
    output.close()


def _save_total_words(words_path):
    words_total_file = words_path + '/zhwikis_total_words.txt'

    words_list, words_number = t_path_file_list(file_path=words_path, suffix=None)

    for words_file in words_list:
        with open(words_file, "r", encoding="utf-8") as f_read:
            with open(words_total_file, "a+", encoding="utf-8") as f_write:
                f_write.write(f_read.read())
    return words_total_file


def _wikiextractor(wiki_text_path):
    output_text_path = './wiki_text/wikiextractor_output_text'
    os.system(f'mkdir -p {output_text_path}')

    os.system(f'python -m wikiextractor.WikiExtractor {wiki_text_path} -o {output_text_path}')
    return output_text_path


def _parse_zhwikis_words(read_file_path, ltp):
    save_file_path = read_file_path + '_words'

    read_file_list, read_file_number = t_path_file_list(file_path=read_file_path, suffix=None)
    progress_bar = t_print_progress_bar(read_file_number)

    for read_file in read_file_list:
        progress_bar.show()
        save_file = save_file_path + '/' + read_file.split('/', 1)[-1].split('/', 1)[-1].rsplit('/', 1)[0]
        os.system(f'mkdir -p {save_file}')
        save_file = save_file + '/' + read_file.rsplit('/', 1)[-1]

        _parse_zhwiki_words(read_file, save_file, ltp)

    return _save_total_words(save_file_path)


def _save_words_file(wiki_text_bz2file, wiki_words_file):
    wiki_text_path = wiki_text_bz2file.rsplit('/', 1)[0]
    wiki_text_name = wiki_text_bz2file.rsplit('/', 1)[-1].rsplit('.', 1)[0] + '.txt'
    wiki_text_file = f'{wiki_text_path}/{wiki_text_name}'

    os.system(f'cp {wiki_words_file} {wiki_text_file}')
    return wiki_text_file


# =====================================================================


def wikiextractor(wiki_text_bz2file):
    ltp = LTP(path = "base")

    print(f'start wikiextractor on {wiki_text_bz2file} ...')
    wiki_result_path = _wikiextractor(wiki_text_bz2file)

    print(f'start Traditional2Simplified on {wiki_result_path} ...')
    wiki_simplified_path = Traditional2Simplified(wiki_result_path)

    print(f'start parse_zhwikis on {wiki_simplified_path} ...')
    wiki_words_file = _parse_zhwikis_words(wiki_simplified_path, ltp)

    print(f'save words to:{wiki_words_file}')
    wiki_text_file = _save_words_file(wiki_text_bz2file, wiki_words_file)

    print(f'save result to:{wiki_text_file}')
    return wiki_text_file


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        wiki_text_bz2file = sys.argv[1]
    else:
        wiki_text_bz2file = '/project/dataset/Public/dumps.wikimedia.org/zhwiki-20210901-pages-articles-multistream.xml.bz2'
    wikiextractor(wiki_text_bz2file)
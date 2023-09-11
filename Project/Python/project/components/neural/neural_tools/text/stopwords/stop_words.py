# -*- coding: utf-8 -*-


def get_stopwords():
    stopword_set = set()
    with open("/project/components/neural/neural_tools/text/stopwords/stop_words.txt", 'r', encoding="utf-8") as stopwords:
        for stopword in stopwords:
            stopword_set.add(stopword.strip('\n'))
    return stopword_set
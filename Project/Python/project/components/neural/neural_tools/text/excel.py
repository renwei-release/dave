#!/usr/bin/python
import sys
import json
import pandas as pd
from components.neural.neural_tools.text.stopwords.stop_words import get_stopwords
from ltp import LTP
from public.tools import *


def _excel_process_base_read(excel_file):
    df = pd.read_excel(excel_file)
    data_frame = pd.DataFrame(df)
    data_frame = data_frame.sample(frac=1).reset_index(drop=True) # 乱序处理
    line_number = data_frame.shape[0]
    return data_frame, line_number


def _excel_process_save_dataset_file(excel_file, data_frame, line_number):
    train_index = int(line_number * 0.9)
    train_frame = data_frame.iloc[0:train_index, :]
    valid_frame = data_frame.iloc[train_index:line_number, :]

    data_file = excel_file.rsplit('.', 1)[0] + '.txt'
    train_file = excel_file.rsplit('.', 1)[0] + '_train.txt'
    valid_file = excel_file.rsplit('.', 1)[0] + '_valid.txt'

    data_frame.to_csv(data_file, header=None, sep=' ', quotechar=' ', index=False)
    train_frame.to_csv(train_file, header=None, sep=' ', quotechar=' ', index=False)
    valid_frame.to_csv(valid_file, header=None, sep=' ', quotechar=' ', index=False)

    return data_file, train_file, valid_file


def _excel_process_save_json_file(excel_file, data_list):
    json_file = excel_file.rsplit('.', 1)[0] + '.json'

    with open(json_file, 'w') as f:
        f.write(json.dumps(data_list, ensure_ascii=False, indent=4, separators=(',', ':')))

    return json_file


def _excel_process_load_fun_name(excel_file):
    return excel_file.rsplit('/', 1)[-1].rsplit('.', 1)[0]


def _excel_process_wuyouxing_customer_service_FAQ(excel_file):
    data_frame, line_number = _excel_process_base_read(excel_file)

    data_list = data_frame['语料'].tolist()
    ltp = LTP(path = "base")
    stopwords = get_stopwords()
    progress_bar = t_print_progress_bar(line_number)
    for line_index, data in enumerate(data_list):
        progress_bar.show()
        words, _ = ltp.seg([data])
        string_word = ''
        for word in words[0]:
            if word not in stopwords:
                string_word += word + ' '
        data_frame.loc[line_index, '语料'] = string_word.strip()

    data_frame['标准问题'] = '__label__' + data_frame['标准问题']
    data_frame[['标准问题', '语料']] = data_frame[['语料', '标准问题']]

    data_file, train_file, valid_file = _excel_process_save_dataset_file(excel_file, data_frame, line_number)

    print(f'data_file:{data_file}')
    print(f'train_file:{train_file}')
    print(f'valid_file:{valid_file}')
    return data_file, train_file, valid_file


def _excel_process_wuyouxing_customer_service_QA(excel_file):
    data_frame, line_number = _excel_process_base_read(excel_file)
    q_list = data_frame['标准问题'].tolist()
    a_list = data_frame['回答'].tolist()
    qa_list = {}

    for index, value in enumerate(q_list):
        qa_list[value] = a_list[index]

    json_file = _excel_process_save_json_file(excel_file, qa_list)

    print(f'json_file:{json_file}')
    return json_file


# =====================================================================


def excel_process(excel_file):
    process_fun_name = _excel_process_load_fun_name(excel_file)

    if process_fun_name == 'wuyouxing_customer_service_FAQ':
        return _excel_process_wuyouxing_customer_service_FAQ(excel_file)
    elif process_fun_name == 'wuyouxing_customer_service_QA':
        return _excel_process_wuyouxing_customer_service_QA(excel_file)
    else:
        print(f'{excel_file} can\'t find process_fun_name!')
        return None, None, None


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        excel_file = sys.argv[1]
    else:
        excel_file = '/project/dataset/Private/wuyouxing_customer_service_FAQ/wuyouxing_customer_service_FAQ.xlsx'
    excel_process(excel_file)
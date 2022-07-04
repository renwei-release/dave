#!/usr/bin/python
import os
import sys


def copy_file(current_file, intent_file):
    print(current_file+'->'+intent_file)

    file_counter = 0
    for curDir, dirs, files in os.walk(current_file):
        for file in files:
            current_file_path = os.path.join(curDir, file)
            file_path_name = str(file[:2]).upper()

            intent_file_path = os.path.join(intent_file, file_path_name)
            if not os.path.exists(intent_file_path):
                os.makedirs(intent_file_path)
            if str(file[:2]) == file_path_name.lower():
                file_counter += 1
                cmd = 'cp  '+current_file_path+' '+intent_file_path
                print(cmd)
                os.system(cmd)
    print(str(file_counter)+' files copy from '+current_file+' to '+intent_file+'!')


if __name__ == '__main__':
    if len(sys.argv) <= 1:
        current_file = None
        intent_file = None
    elif len(sys.argv) <= 2:
        current_file = sys.argv[1]
        intent_file = None
    else:
        current_file = sys.argv[1]
        intent_file = sys.argv[2]

    if current_file == None:
        current_file = '/home/Data-Center/dataset/Private/PhotographicAesthetics/total'
    if intent_file == None:
        intent_file = '/home/ouyang/tools/fastdfs/storage/storage/data/FD'

    copy_file(current_file, intent_file)
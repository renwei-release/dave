#!/usr/bin/python
import re
import os
import sys,getopt

# 进行哈希加密
from hashlib import sha1


def hash(datas):
    sha = sha1()
    sha.update(str(datas).encode('utf-8'))  # 进行加密
    hashcode = sha.hexdigest()  # 获取加密字符串
    return hashcode


def file_path(path):
    file_list = os.listdir(path)
    for bag in file_list:
        join_file_path = os.path.join(path, bag)
        if os.path.isdir(join_file_path):
            image_file_name = os.listdir(join_file_path)
            for file in image_file_name:
                image_file_path = os.path.join(join_file_path, file)
                os.replace(image_file_path,join_file_path+'/'+hash(str(file).split('.')[0])+'.'+str(file).split('.')[-1])
                print(image_file_path)


def main(args):
    file_path(args[0])


if __name__ == '__main__':
    main(sys.argv[1:])


"""
用于重命名图片文件名称: 用hash重新命名
"""
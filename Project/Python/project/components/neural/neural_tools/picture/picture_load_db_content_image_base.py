#!/usr/bin/python
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.12.29.
# ================================================================================
#
import pymysql
import json
from public.tools import *


def _db_content_image_base_conn():
    return pymysql.connect(
        host = '127.0.0.1',
        port = 3306,
        user = 'root',
        password = 'CWLtc14@#!',
        db = 'DAVEDB0005',
        charset="utf8")


def _db_content_image_base_json(db_cursor, image_id):
    db_cursor.execute(f'SELECT image_id, create_time, dimensions, title, author, introduction FROM content_image_base WHERE image_id = \"{image_id}\";')
    result = db_cursor.fetchone()
    if result:
        db_json = {
            'image_id': result[0],
            'create_time': result[1],
            'dimensions': result[2],
            'title': result[3],
            'author': result[4],
            'introduction': result[5],
        }
        return db_json
    else:
        return None


def _db_content_image_base_write(db_opt, write_file, image_id):
    db_cursor = db_opt.cursor()

    string_json = _db_content_image_base_json(db_cursor, image_id)

    if string_json != None:
        with open(write_file, "a+", encoding="utf-8") as f:
            json.dump(string_json, f, ensure_ascii=False)
            f.write('\r\n')
    return


def _db_content_image_base_path(db_opt, path_child):
    file_list, file_number = t_path_file_list(path_child)
    write_file = None
    for file_name in file_list:
        if write_file == None:
            write_file = file_name.rsplit('.', 1)[0] + '.txt'
        
        image_id = file_name.rsplit('/', 1)[-1].rsplit('.', 1)[0]

        _db_content_image_base_write(db_opt, write_file, image_id)
    return


# =====================================================================


def picture_load_db_content_image_base(file_path):
    db_opt = _db_content_image_base_conn()

    path_list = t_path_child_list(file_path)
    path_number = len(path_list)

    progress_bar = t_print_progress_bar(path_number)

    for path_child in path_list:
        _db_content_image_base_path(db_opt, path_child)
        progress_bar.show()

    return


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        file_path = sys.argv[1]
    else:
        file_path = './picture_find_similar_image'
    picture_load_db_content_image_base(file_path)
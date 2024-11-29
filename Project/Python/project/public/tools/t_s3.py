# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from .t_rand import t_rand
import os


# =====================================================================


def t_s3_setup_s3_client():
    if os.path.exists("/root/.obsutilconfig") == False:
        os.system("chmod +x /obsutil/setup-obsutil.sh && /obsutil/setup-obsutil.sh")
    return


def t_s3_is_s3_path(path):
    return path.startswith('obs://') or path.startswith('s3://') or path.startswith('oss://') or path.startswith('cos://')


def t_s3_is_valid_path(path):
    if not t_s3_is_s3_path(path):
        return False

    t_s3_setup_s3_client()

    ret = os.system(f'chmod +x /obsutil/obsutil && /obsutil/obsutil stat {path} > /dev/null 2>&1 && echo $?')
    return ret == 0


def t_s3_input_file_download(input_file):
    t_s3_setup_s3_client()

    if t_s3_is_s3_path(input_file):
        input_download_file = f"/dave/{t_rand()}_{os.path.basename(input_file)}"
        os.system(f"chmod +x /obsutil/obsutil && /obsutil/obsutil cp {input_file} {input_download_file}")
    else:
        input_download_file = input_file
    return input_download_file


def t_s3_input_file_remove(download_file):
    if download_file.startswith('/dave/'):
        os.remove(download_file)
    return


def t_s3_output_file_change(output_file):
    if t_s3_is_s3_path(output_file):
        output_local_file = f"/dave/{t_rand()}_{os.path.basename(output_file)}"
    else:
        output_local_file = output_file
    return output_local_file


def t_s3_output_file_upload(output_local_file, output_file):
    t_s3_setup_s3_client()

    if t_s3_is_s3_path(output_file):
        os.system(f"chmod +x /obsutil/obsutil && /obsutil/obsutil cp {output_local_file} {output_file}")
        os.remove(f"{output_local_file}")
    return
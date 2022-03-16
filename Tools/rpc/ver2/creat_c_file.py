# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2022 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2022.01.19.
#
import os


# =====================================================================


def creat_c_file(proto_file):
    c_path = proto_file.rsplit('/', 1)[0]

    os.system(f'protoc-c --c_out={c_path} --proto_path={c_path} {proto_file}')
    return
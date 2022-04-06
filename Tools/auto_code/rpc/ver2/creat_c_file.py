# -*- coding: utf-8 -*-
#
# Copyright (c) 2022 Renwei
#
# This is a free software; you can redistribute it and/or modify
# it under the terms of the MIT license. See LICENSE for details.
#
import os


# =====================================================================


def creat_c_file(proto_file):
    c_path = proto_file.rsplit('/', 1)[0]

    os.system(f'protoc-c --c_out={c_path} --proto_path={c_path} {proto_file}')
    return
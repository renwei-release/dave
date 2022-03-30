# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import os
import sys


def t_copy_byte_to_array(dst, src):
    index = 0
    for src_data in src:
        dst[index] = src_data
        index += 1
    return
# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import random


# =====================================================================


def t_string_hash(string_data):
    string_hash = hex(int(hash(string_data)))[2:]
    return string_hash
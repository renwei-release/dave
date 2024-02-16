# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import hashlib


def _string_hash(input_string):
    return hashlib.sha512(input_string.encode()).hexdigest()


# =====================================================================


def t_string_hash(string_data):
    if isinstance(string_data, str) == False:
        string_data = str(string_data)
    return _string_hash(string_data)
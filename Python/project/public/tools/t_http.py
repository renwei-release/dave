# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from ctypes import *


def t_http_get_head_value(head, key):
    for key_value in head:
        if key_value.key == key:
            return key_value.value
    return None


def t_http_request_uri(head):
    return t_http_get_head_value(head, b'REQUEST_URI')
# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import base64


def t_crypto_base64_encode(data):
    if isinstance(data, str):
        raw = data.encode('utf-8')
    elif isinstance(data, (bytes, bytearray, memoryview)):
        raw = bytes(data)
    else:
        raise TypeError('data must be str or bytes-like')

    return base64.b64encode(raw).decode('utf-8')


def t_crypto_base64_decode(data):
    if isinstance(data, bytes):
        decoded = base64.b64decode(data)
    else:
        decoded = base64.b64decode(data.encode('utf-8'))

    try:
        return decoded.decode('utf-8')
    except UnicodeDecodeError:
        return decoded
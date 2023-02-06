# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import random
import string


def t_rand():
    return ''.join(random.sample(string.ascii_letters + string.digits, 16))
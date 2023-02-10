# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from public import *
import requests


# =====================================================================


def get(uri):
    rsp = requests.get(uri)
    return rsp
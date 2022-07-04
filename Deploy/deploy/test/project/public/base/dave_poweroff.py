# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from ctypes import *
from string import *
from .dave_dll import *


davelib=dave_dll()


# =====================================================================


def dave_poweroff():
    davelib.dave_dll_poweroff()
    return
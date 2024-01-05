# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from ctypes import *
from string import *
from .dave_dll import *


davelib=dave_dll()


# =====================================================================


def dave_system_online():
    davelib.dave_dll_system_online()
    return


def dave_system_offline():
    davelib.dave_dll_system_offline()
    return
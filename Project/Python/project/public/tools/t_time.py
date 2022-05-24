# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import os
import sys
import time


def t_time_current_str():
    time_tuple = time.localtime(time.time())

    year = "%04d" % time_tuple[0]
    month = "%02d" % time_tuple[1]
    day = "%02d" % time_tuple[2]
    hour = "%02d" % time_tuple[3]
    minute = "%02d" % time_tuple[4]
    second = "%02d" % time_tuple[5]

    return year+month+day+hour+minute+second
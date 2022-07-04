# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.06.07.
# ================================================================================
#
from components.busybox import *


CV_THREAD_NAME=b'cv'


def cv_register(function_id):
    busybox_APPMSG_FUNCTION_REGISTER_REQ(CV_THREAD_NAME, function_id)
    return
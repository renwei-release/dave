# -*- coding: utf-8 -*-
#
# ================================================================================
# (c) Copyright 2021 Renwei All rights reserved.
# --------------------------------------------------------------------------------
# 2021.06.07.
# ================================================================================
#
from components.busybox import *


AIB_THREAD_NAME=b'main_aib'


def aib_register(function_id):
    busybox_APPMSG_FUNCTION_REGISTER_REQ(AIB_THREAD_NAME, function_id)
    return
# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from public import *
from components.bdata import *


def _bdata_debug():
    BDATALOG("TEST", f"bdata debug : {222}")
    return "bdata debug"


def _rcfg_add_debug():
    rcfg_set("/add_debug", "aaaaaaaaaaaaaaa")
    return "rcfg add debug"


def _rcfg_get_debug():
    cfg_value = rcfg_get("/add_debug")
    DAVELOG(f"cfg_value:{cfg_value}")
    return ""


def _rcfg_del_debug():
    if rcfg_del("/add_debug") == 0:
        return "rcfg del OK!"
    else:
        return "rcfg del success!"


# =====================================================================


def base_debug(req_msg):
    DAVELOG(f"{req_msg}")

    if req_msg == "bdata":
        return _bdata_debug()
    elif req_msg == "rcfg_add":
        return _rcfg_add_debug()
    elif req_msg == "rcfg_get":
        return _rcfg_get_debug()
    elif req_msg == "rcfg_del":
        return _rcfg_del_debug()

    return f"invalid req {req_msg}"
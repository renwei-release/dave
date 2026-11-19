# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from public import *
from components.bdata import *
from components.store import *


def _bdata_debug():
    BDATALOG("TEST", f"bdata debug : {222}")
    return "bdata debug"


def _store_debug():
    database = "DAVEDBTEST"

    STOREMYSQL(f"CREATE DATABASE {database} charset=utf8;")

    STOREMYSQL(f"CREATE TABLE {database}.test (id int primary key auto_increment, device_id int not null, name char(128), password char(64), email char(128), updatetime timestamp default current_timestamp, constraint unique(name));")
    return ""


def _co_debug():

    rsp = dave_call_general("SEARCH", "search-inq", {
        "collection": "test",
        "question": "zhu zhiqing",
        "historys": [],
        "top_k": 3,
        "offset": 0
    })

    return f"co debug : {rsp}"


# =====================================================================


def base_debug(req_msg):
    DAVELOG(f"{req_msg}")

    if req_msg == "bdata":
        return _bdata_debug()
    elif req_msg == "store":
        return _store_debug()
    elif req_msg == "co":
        return _co_debug()

    return f"invalid req {req_msg}"
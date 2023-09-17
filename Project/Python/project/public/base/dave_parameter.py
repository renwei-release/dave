# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from .dave_cfg import *


GLOBALLY_IDENTIFIER_CFG_NAME = "GLOBALLYIDENTIFIER"


def _globally_identifier():
    global GLOBALLY_IDENTIFIER_CFG_NAME
    return cfg_get(GLOBALLY_IDENTIFIER_CFG_NAME, "")
__globally_identifier__ = _globally_identifier()


# =====================================================================


def globally_identifier():
    global __globally_identifier__
    if __globally_identifier__ == "":
        __globally_identifier__ = _globally_identifier()
    return __globally_identifier__
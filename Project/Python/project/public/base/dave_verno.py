# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import platform
import time
import sys


default_PRODUCT="defaultBASE"


class define_VERSION:
    VERSION_PRODUCT=default_PRODUCT
    VERSION_MISC=("".join(("py"+platform.python_version()).replace('.', '-')))
    VERSION_MAIN="5"
    VERSION_SUB="2"
    VERSION_REV="11"
    VERSION_DATE_TIME=time.strftime("%Y%m%d%H%M%S", time.localtime())
    VERSION_LEVEL="Alpha"


v = define_VERSION()


def _reset_product():
   global v
   if v.VERSION_PRODUCT == default_PRODUCT:
      if len(sys.argv) >= 2:
         v.VERSION_PRODUCT = f'{sys.argv[1]}'

   v.VERSION_PRODUCT = v.VERSION_PRODUCT.upper()

   return (v.VERSION_PRODUCT).encode("utf-8")


def _dave_project() -> str:
    raw = _reset_product()
    if isinstance(raw, (bytes, bytearray)):
        return raw.decode("utf-8", errors="ignore").strip().lower()
    return str(raw).strip().lower()


__project__ = _dave_project()
__PROJECT__ = __project__.upper()

# =====================================================================


def dave_product():
   return _reset_product()


def dave_project() -> str:
   return __project__


def dave_PROJECT() -> str:
   return __PROJECT__


def dave_verno():
   global v
   _reset_product()
   return (v.VERSION_PRODUCT+"."+v.VERSION_MISC+"."+v.VERSION_MAIN+"."+v.VERSION_SUB+"."+v.VERSION_REV+"."+v.VERSION_DATE_TIME+"."+v.VERSION_LEVEL).encode("utf-8")
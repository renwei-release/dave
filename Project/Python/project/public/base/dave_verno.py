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
    VERSION_MAIN="4"
    VERSION_SUB="13"
    VERSION_REV="10"
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


# =====================================================================


def dave_product():
   return _reset_product()


def dave_verno():
   global v
   _reset_product()
   return (v.VERSION_PRODUCT+"."+v.VERSION_MISC+"."+v.VERSION_MAIN+"."+v.VERSION_SUB+"."+v.VERSION_REV+"."+v.VERSION_DATE_TIME+"."+v.VERSION_LEVEL).encode("utf-8")
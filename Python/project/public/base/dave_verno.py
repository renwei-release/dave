# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import platform
import time


class define_VERSION:
    VERSION_PRODUCT="DEMO"
    VERSION_MISC=("".join(("py"+platform.python_version()).replace('.', '-')))
    VERSION_MAIN="4"
    VERSION_SUB="5"
    VERSION_REV="5"
    VERSION_DATE_TIME=time.strftime("%Y%m%d%H%M%S", time.localtime())
    VERSION_LEVEL="Alpha"


v = define_VERSION()


# =====================================================================


def dave_product(product_name=None):
   global v
   if product_name != None:
      v.VERSION_PRODUCT = f'{product_name}'.upper()
   return (v.VERSION_PRODUCT).encode("utf-8")


def dave_verno():
   global v
   return (v.VERSION_PRODUCT+"."+v.VERSION_MISC+"."+v.VERSION_MAIN+"."+v.VERSION_SUB+"."+v.VERSION_REV+"."+v.VERSION_DATE_TIME+"."+v.VERSION_LEVEL).encode("utf-8")
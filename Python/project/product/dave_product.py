# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import importlib
import traceback
import sys
import public


def _setup_product(product_name):
   public.dave_product(product_name)
   return


# =====================================================================


def dave_product(product_name):
    _setup_product(product_name)

    try:
        sys.path.append("/project/product")
        return importlib.import_module(f'{product_name}.{product_name}_function')
    except:
        print(f"{product_name} does not exist or has wrong!!!!!! the sys.path:{sys.path}")
        traceback.print_exc()
        return None        
# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import importlib
import traceback


# =====================================================================


def dave_import_product(product_name):
    try:
        return importlib.import_module(f'product.{product_name}.{product_name}_product')
    except:
        print(f"{product_name} does not exist or has wrong!!!!!!")
        traceback.print_exc()
        return None
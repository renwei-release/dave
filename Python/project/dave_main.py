# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import sys
import dave
from product.product_function import *


# =====================================================================


def main():
   product_name = sys.argv[1]

   product = product_function(product_name)

   if product != None:
      dave.dave_python_init(product.product_init, product.product_exit)
   else:
      dave.dave_python_init(None, None)
   dave.dave_python_running()
   dave.dave_python_exit()
   return


if __name__ == "__main__":
   main()
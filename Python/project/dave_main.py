# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import sys
import public
from product.dave_product import *


# =====================================================================


def main():
   product_name = sys.argv[1]

   product = dave_product(product_name)

   if product != None:
      public.dave_python_init(product.dave_product_init, product.dave_product_exit)
   else:
      public.dave_python_init(None, None)
   public.dave_python_running()
   public.dave_python_exit()
   return


if __name__ == "__main__":
   main()
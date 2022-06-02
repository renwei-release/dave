# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import os
from ctypes import *
from .dave_verno import *
from .dave_msg_function import *
from product.dave_product import *


class DllMsgBody(Structure):
    _fields_ = [
        ("msg_src_name", c_char * 128),
        ("msg_src", c_longlong),
        ("msg_dst_name", c_char * 128),
        ("msg_dst", c_longlong),
        ("msg_id", c_longlong),
        ("msg_len", c_longlong),
        ("msg_check", c_longlong),
        ("msg_body", c_void_p)
    ]


path=os.path.dirname(os.path.abspath(__file__))
davelib=cdll.LoadLibrary(path+"/lib/liblinuxBASE.so")


_my_product_name = None
_product_init_fun = None
_product_exit_fun = None


def _python_import_product(product_name):
   global _my_product_name
   global _product_init_fun
   global _product_exit_fun

   if _product_init_fun == None:
      product = dave_import_product(_my_product_name)
      if product != None:
         _product_init_fun = product.dave_product_init
         _product_exit_fun = product.dave_product_exit
   return


INITFUNC=CFUNCTYPE(None, POINTER(c_void_p))
def _python_init(NULL_DATA):
   _python_import_product(_my_product_name)

   if _product_init_fun != None:
      _product_init_fun()
   return


MAINFUNC=CFUNCTYPE(None, POINTER(DllMsgBody))
def _python_main(msg):
   dave_msg_process(msg)
   return


EXITFUNC=CFUNCTYPE(None, POINTER(c_void_p))
def _python_exit(NULL_DATA):
   if _product_exit_fun != None:
      _product_exit_fun()
   return


CHECKFUNC=CFUNCTYPE(c_int, c_int)
def _dll_python_self_check_test_callback(value):
    return value


_c_python_init = INITFUNC(_python_init)
_c_python_main = MAINFUNC(_python_main)
_c_python_exit = INITFUNC(_python_exit)
_c_dll_python_self_check_test_callback = CHECKFUNC(_dll_python_self_check_test_callback)


def _dll_python_self_check():
   if davelib.dave_dll_self_check(c_char_p(b"123456"), c_int(123456), c_float(123456.123456), _c_dll_python_self_check_test_callback) == 0:
      return True
   return False


def _dll_setup_product_verno_name(product_name):
   dave_product(product_name)
   return


# =====================================================================


def dave_dll():
   return davelib


def dave_python_init(product_name):
   global _my_product_name

   _my_product_name = str(product_name)

   _dll_setup_product_verno_name(_my_product_name)

   my_verno = dave_verno()
   model = b"Outer Loop"
   thread_number = 3

   davelib.dave_dll_init(c_char_p(my_verno), c_char_p(model), c_int(thread_number), _c_python_init, _c_python_main, _c_python_exit)
   return


def dave_python_running():
   ret = _dll_python_self_check()
   if ret == True:
      davelib.dave_dll_running()
   davelib.dave_dll_wait_dll_exit()
   return


def dave_python_exit():
   davelib.dave_dll_exit()
   return
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
        ("msg_src_gid", c_char * 64),
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


_product_init_fun = None
_product_exit_fun = None


def _python_import_product():
   global _product_init_fun
   global _product_exit_fun

   if _product_init_fun == None:
      product = dave_import_product(dave_product())
      if product != None:
         _product_init_fun = product.dave_product_init
         _product_exit_fun = product.dave_product_exit
   return


INITFUNC=CFUNCTYPE(None, POINTER(c_void_p))
def _python_init(NULL_DATA):
   _python_import_product()
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


def _product_cfg():
   product_name = dave_product().decode().lower()

   cfg = None
   try:
      cfg = importlib.import_module(f'product.{product_name}.{product_name}_cfg')
   except:
      cfg = None

   if cfg != None:
      thread_number, work_mode, product_name = cfg.dave_product_cfg()
   else:
      thread_number, work_mode, product_name = 1, b"Outer Loop", b''

   if (product_name != None) and (type(product_name) != bytes):
      product_name = product_name.encode()

   return thread_number, work_mode, product_name


CHECKFUNC=CFUNCTYPE(c_int, c_int)
def _python_self_check_test_callback(value):
    return value


_c_python_init = INITFUNC(_python_init)
_c_python_main = MAINFUNC(_python_main)
_c_python_exit = INITFUNC(_python_exit)
_c_dll_python_self_check_test_callback = CHECKFUNC(_python_self_check_test_callback)


def _python_self_check():
   if davelib.dave_dll_self_check(c_char_p(b"123456"), c_int(123456), c_float(123456.123456), _c_dll_python_self_check_test_callback) == 0:
      return True
   return False


# =====================================================================


def dave_python_init():
   thread_number, work_mode, product_name = _product_cfg()
   my_verno = dave_verno()
   sync_domain = b""

   davelib.dave_dll_init(
      c_char_p(product_name),
      c_char_p(my_verno), c_char_p(work_mode),
      c_int(thread_number),
      _c_python_init, _c_python_main, _c_python_exit,
      c_char_p(sync_domain))
   return


def dave_python_running():
   ret = _python_self_check()
   if ret == True:
      davelib.dave_dll_running()
   return


def dave_python_exit():
   davelib.dave_dll_exit()
   return


def dave_dll():
   return davelib
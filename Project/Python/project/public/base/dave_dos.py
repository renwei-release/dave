# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from ctypes import *
from .dave_dll import dave_dll


davelib = dave_dll()


# =====================================================================


DOSREGFUNC=CFUNCTYPE(c_int, c_char_p, c_int)
def dos_cmd_reg(cmd, reg_fun):
    byte_cmd = bytes(cmd, encoding="utf8")
    davelib.dave_dll_dos_cmd_reg.restype = c_int
    davelib.dave_dll_dos_cmd_reg(c_char_p(byte_cmd), reg_fun)
    return


def dos_print(*msg: object):
    msg = str(msg[0]).encode("utf-8")
    davelib.dave_dll_dos_print(c_char_p(msg))
    return


def dos_get_user_input(*give_user_msg: object):
    wait_second = 360

    give_user_msg = str(give_user_msg[0]).encode("utf-8")
    davelib.dave_dll_dos_get_user_input.restype = c_char_p
    ret = davelib.dave_dll_dos_get_user_input(c_char_p(give_user_msg), c_int(wait_second))
    return ret.decode("utf-8").strip()
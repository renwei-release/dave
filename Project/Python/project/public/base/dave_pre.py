# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from .dave_dll import dave_dll
from .dave_verno import *


davelib = dave_dll()


def _reset_verno():
   davelib.dave_dll_reset_verno(dave_verno())
   return


# =====================================================================


def dave_python_system_pre_init():
	#/*
	# * Preventing the system from being called in advance
	# * without initialization call
	# */
   _reset_verno()
   return


dave_python_system_pre_init()
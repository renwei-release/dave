# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from .dave_system_function import *


def dave_msg_process(msg):
   fun = system_function_table.get(msg.contents.msg_id, None)
   if fun != None:
      fun(msg.contents.msg_src_name, msg.contents.msg_src, msg.contents.msg_len, msg.contents.msg_body)
   else:
      print(f"unprocess msg, {msg.contents.msg_src_name}->{msg.contents.msg_dst_name}:{msg.contents.msg_id}")
   return
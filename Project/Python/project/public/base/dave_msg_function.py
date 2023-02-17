# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from ..tools import *
from .dave_system_function import dave_system_function_table_inq


# =====================================================================


def dave_msg_process(msg):
   fun = dave_system_function_table_inq(msg.contents.msg_id)
   if fun != None:
      fun(msg.contents.msg_src_name, msg.contents.msg_src, msg.contents.msg_len, msg.contents.msg_body)
   else:
      print(f"unprocess msg, {msg.contents.msg_src_name}->{msg.contents.msg_dst_name}:{t_audo_RPCMSG_str(msg.contents.msg_id)}")
   return
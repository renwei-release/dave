# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

#
# 这是一个想用protobuf传输RPC的尝试，但看起来失败了，原因如下：
# 1，protobuf-c没有办法很好的处理c里面的联合体，
#    有一个oneof类似看起来可行，但需要在c这边按protobuf的格式传递
#    联合体数据，修改代码量会比较多，目前不现实。具体可参考：
#    https://github.com/protobuf-c/protobuf-c/wiki/Examples#oneofs
# 2，protobuf-c是以整个c的结构体处理打包和解包的，如果c的结构体里面有特
#    别的内存指针数据需要处理时，比如MBUF结构，没有办法单独灵活的对内存
#    指针做释放或申请的处理。
# 3，protobuf-c对结构体的成员是按序号处理的，如果新旧版本成员有变化，比如新
#    版本去掉了一个成员A，那么兼容的方式是A位置的序号不变，比如A的需要是2，那么
#    新版本里面可以不用A，但序号要保留，最终新版本需要为1，3，4。。。。。。
#    这样没法做成自动化，需要手动来修改proto文件。更复杂的情况是成员变量名称改变
#    时 ，更难处理。
#

import os
from rpc_cfg import *
from ver2.creat_proto_file import creat_proto_file
from ver2.creat_c_file import creat_c_file


def rpc_ver2():
    if not os.path.exists(ver2_auto_dir):
        os.makedirs(ver2_auto_dir)

    proto_file = creat_proto_file()
    creat_c_file(proto_file)
    return
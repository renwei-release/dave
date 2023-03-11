# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from find.find_param import find_param
from rpc.rpc_main import rpc_main
from enumstr.enumstr_main import enumstr_main


# =====================================================================


def autocode_main():
    param = find_param()
    if param == None:
        print("The header file has not changed and there is no need to rebuild the automation code!")
        return

    rpc_main(param)
    enumstr_main(param)


if __name__ == '__main__':
    autocode_main()
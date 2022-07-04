# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import pickle


# =====================================================================


def t_class_save(file_path, class_body):
    output_hal = open(file_path, 'wb')
    str = pickle.dumps(class_body)
    output_hal.write(str)
    output_hal.close()
    return


def t_class_load(file_path):
    class_body = None
    with open(file_path, 'rb') as file:
        class_body  = pickle.loads(file.read())
    return class_body
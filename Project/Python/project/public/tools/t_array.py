# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */


# =====================================================================


def t_array_add(arr1, arr2):
    if len(arr1) == 0:
        return arr2

    if len(arr1) != len(arr2):
        print(f"t_array_add len(arr1):{len(arr1)} != len(arr2):{len(arr2)}")
        return None

    arr = []
    for i in range(len(arr1)):
        arr.append(arr1[i] + arr2[i])
    return arr


def t_array_sub(arr1, arr2):
    if len(arr1) == 0:
        return arr2

    if len(arr1) != len(arr2):
        print(f"t_array_sub len(arr1):{len(arr1)} != len(arr2):{len(arr2)}")
        return None

    arr = []
    for i in range(len(arr1)):
        arr.append(arr1[i] - arr2[i])
    return arr


def t_array_div(arr, dividend):
    if len(arr) == 0:
        return arr

    div_arr = []
    for i in range(len(arr)):
        div_arr.append(arr[i] / dividend)
    return div_arr
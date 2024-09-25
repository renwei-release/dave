# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import time
import datetime


def t_time_year_month_day():
    time_tuple = time.localtime(time.time())

    year = time_tuple[0]
    month = time_tuple[1]
    day = time_tuple[2]

    return year, month, day


def t_time_week():
    now = datetime.datetime.now()

    week = now.isocalendar()[1]
    weekday = now.weekday() + 1
    return week, weekday


def t_time_current_str():
    time_tuple = time.localtime(time.time())

    year = "%04d" % time_tuple[0]
    month = "%02d" % time_tuple[1]
    day = "%02d" % time_tuple[2]
    hour = "%02d" % time_tuple[3]
    minute = "%02d" % time_tuple[4]
    second = "%02d" % time_tuple[5]

    return year+month+day+hour+minute+second


def t_time_current_us():
    seconds_since_epoch = time.time()

    microseconds_since_epoch = seconds_since_epoch * 1000000
    return int(microseconds_since_epoch)


def t_time_current_ms():
    seconds_since_epoch = time.time()

    milliseconds_since_epoch = seconds_since_epoch * 1000
    return int(milliseconds_since_epoch)


def t_time_current_s():
    seconds_since_epoch = time.time()

    return int(seconds_since_epoch)


def t_time_start_action():
    return datetime.datetime.now()


def t_time_end_action(start_time, time_flag=None, time_name=None, time_msg=None):
    stop_time = datetime.datetime.now()

    run_time = stop_time - start_time 
    run_time = run_time.seconds * 1000000 + run_time.microseconds

    if run_time <= 0:
        run_time_msg = f'0s'
    elif run_time < 1000:
        run_time_msg = f'{run_time}us'
    elif run_time < 1000000:
        run_time_msg = f'{run_time/1000}ms'
    else:
        # detected bug, maybe date updated.
        if run_time > 1000000 * 1000:
            print(f't_time_end_action run_time:{run_time} is too long start_time:{start_time} stop_time:{stop_time}')
        run_time_msg = f'{run_time/1000000}s'

    if time_msg != None:
        run_time_msg = run_time_msg + '-' + time_msg

    if (time_flag != None) and (time_name != None):
        if time_name in time_flag:
            time_flag[time_name] = time_flag[time_name] + '/' + run_time_msg
        else:
            time_flag[time_name] = run_time_msg

    return run_time_msg
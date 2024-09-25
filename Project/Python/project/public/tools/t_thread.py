# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import threading


def _wait_thread_run(thread, timeout):
    if timeout == 0:
        return

    try:
        thread.join(timeout=timeout)
    except:
        pass

    if thread.is_alive():
        thread._tstate_lock.release()
        thread._stop()
    return


# =====================================================================


def t_thread_run(function, args, timeout=8):
    thread = threading.Thread(target=function, args=args)
    thread.start()
    _wait_thread_run(thread, timeout)
    return


def t_thread_run_no_wait(function, args):
    t_thread_run(function, args, 0)
    return
# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
import queue
import threading


# =====================================================================


def t_thread_run(function, args, timeout=8):
    thread = threading.Thread(target=function, args=args)
    thread.start()

    try:
        thread.join(timeout=timeout)
    except:
        pass

    if thread.is_alive():
        thread._tstate_lock.release()
        thread._stop()

    return
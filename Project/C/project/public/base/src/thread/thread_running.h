/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_RUNNING_H__
#define __THREAD_RUNNING_H__
#include "base_macro.h"
#include "dave_base.h"
#include "thread_struct.h"

void thread_running(ThreadStack **ppCurrentMsgStack, base_thread_fun thread_fun, ThreadStruct *pThread, MSGBODY *msg, dave_bool enable_stack);

sb thread_running_cfg_life(void);

#endif


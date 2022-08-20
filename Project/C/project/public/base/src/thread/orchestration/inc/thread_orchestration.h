/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_ORCHESTRATION_H__
#define __THREAD_ORCHESTRATION_H__
#include "base_macro.h"
#include "dave_base.h"
#include "thread_struct.h"

void thread_orchestration_init(void);

void thread_orchestration_exit(void);

dave_bool thread_orchestration_router(ThreadRouter *pRouter, s8 *uid);

#endif


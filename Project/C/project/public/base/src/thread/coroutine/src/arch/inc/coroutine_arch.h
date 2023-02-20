/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __COROUTINE_ARCH_H__
#define __COROUTINE_ARCH_H__

#if defined(__x86_64__)
#include "coroutine_x86_swap.h"
#elif defined(__aarch64__)
#include "coroutine_arm_swap.h"
#else
#error Please define the contents of the chip architecture !!!
#endif

#endif



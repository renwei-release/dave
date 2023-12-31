/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_PROTECTOR_H__
#define __THREAD_PROTECTOR_H__
#include "dave_base.h"

void thread_protector_reg(ThreadId thread_id);
void thread_protector_unreg(ThreadId thread_id);

#endif


/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __IO_TEST_H__
#define __IO_TEST_H__
#include "dave_base.h"

void io_echo(ThreadId src, MsgIdEcho *pEcho);

void io_debug(ThreadId src, DebugReq *pReq);

#endif


/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_TEST_H__
#define __BASE_TEST_H__
#include "dave_base.h"

void base_echo(ThreadId src, MsgIdEcho *pEcho);

void base_debug(ThreadId src, DebugReq *pReq);

#endif


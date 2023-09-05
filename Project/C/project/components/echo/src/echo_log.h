/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __ECHO_LOG_H__
#define __ECHO_LOG_H__
#include "dave_base.h"

#define ECHOLOG(a, ...) { DAVELOG("[ECHO]<%s:%d>", __func__, __LINE__); DAVELOG((const char*)a, ##__VA_ARGS__); DAVELOG("\n"); }

#endif


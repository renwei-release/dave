/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DOS_SHOW_H__
#define __DOS_SHOW_H__
#include "dave_base.h"

void dos_show_init(void);

void dos_show_exit(void);

void dos_print(const char *fmt, ...);

void dos_write(const char *fmt, ...);

#endif


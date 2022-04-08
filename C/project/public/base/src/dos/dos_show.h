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

void dos_view(s8 *msg);

void dos_show(s8 *msg);

void dos_print(const char *fmt, ...);

void dos_show_prompt(void);

#endif

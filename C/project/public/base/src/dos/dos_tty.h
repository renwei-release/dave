/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DOS_TTY_H__
#define __DOS_TTY_H__
#include "dave_base.h"

void dos_tty_init(void);

void dos_tty_exit(void);

void dos_tty_write(u8 *msg, ub msg_len);

#endif


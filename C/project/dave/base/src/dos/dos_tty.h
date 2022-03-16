/*
 * ================================================================================
 * (c) Copyright 2016 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2017.01.20.
 * ================================================================================
 */

#ifndef __DOS_TTY_H__
#define __DOS_TTY_H__
#include "dave_base.h"

void dos_tty_init(void);

void dos_tty_exit(void);

void dos_tty_write(u8 *msg, ub msg_len);

#endif


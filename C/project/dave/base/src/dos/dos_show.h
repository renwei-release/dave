/*
 * ================================================================================
 * (c) Copyright 2016 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
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


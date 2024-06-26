/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_OS_TTY_H__
#define __DAVE_OS_TTY_H__

dave_bool dave_os_tty_init(sync_notify_fun notify_fun);

void dave_os_tty_exit(void);

void dave_os_tty_write(u8 *data_ptr, ub data_len);

ub dave_os_tty_read(u8 *data_ptr, ub data_len);

ub dave_os_tty_get(u8 *data_ptr, ub data_len, ub wait_second);

void dave_os_trace(TraceLevel level, ub data_len, s8 *data_ptr);

#endif


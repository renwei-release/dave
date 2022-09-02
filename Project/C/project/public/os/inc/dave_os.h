/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_OS_H__
#define __DAVE_OS_H__
#include "dave_base.h"

#ifdef __cplusplus
extern "C"{
#endif

#define TIMER_SIG    (SIGALRM)
#define QUIT_SIG     (SIGUSR1)
#define BREAK_SIG    (SIGRTMIN)
#define IO_SIG       (SIGIO)
#define KILL_SIG     (SIGTERM)

#include "dave_os_api.h"
#include "dave_os_file.h"
#include "dave_os_socket.h"
#include "dave_os_thread.h"
#include "dave_os_time.h"
#include "dave_os_tty.h"
#include "dave_os_info.h"

#ifdef __cplusplus
}
#endif
#endif


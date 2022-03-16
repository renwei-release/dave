/*
 * ================================================================================
 * (c) Copyright 2016 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#ifndef __DAVE_LINUX_H__
#define __DAVE_LINUX_H__
#include <signal.h>
#include "dave_os.h"

#define TIMER_SIG    (SIGALRM)
#define QUIT_SIG     (SIGUSR1)
#define BREAK_SIG    (SIGRTMIN)
#define IO_SIG       (SIGIO)
#define KILL_SIG     (SIGTERM)

void dave_os_timer_notify(unsigned long data);

#endif


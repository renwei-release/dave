/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_QUIT_H__
#define __THREAD_QUIT_H__

typedef enum {
	QUIT_TYPE_RESTART = 0,
	QUIT_TYPE_POWER_OFF,
	QUIT_TYPE_MAX
} QUITTYPE;

void thread_quit_init(void *main_thread_id);

void thread_quit_exit(void);

void thread_quit(QUITTYPE type, s8 *reason, ThreadStruct *task, ub task_num);

#endif


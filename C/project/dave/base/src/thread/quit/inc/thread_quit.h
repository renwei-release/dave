/*
 * ================================================================================
 * (c) Copyright 2016 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
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


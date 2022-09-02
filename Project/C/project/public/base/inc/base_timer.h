/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_TIMER_H__
#define __BASE_TIMER_H__

#define INVALID_TIMER_ID (-1)

typedef sb TIMERID;
typedef void (* base_timer_fun)(TIMERID timer_id, ub thread_index);
typedef void (* base_timer_param_fun)(TIMERID timer_id, ub thread_index, void *param_ptr);

TIMERID base_timer_creat(char *name, base_timer_fun fun, ub alarm_ms);
TIMERID base_timer_param_creat(char *name, base_timer_param_fun fun, void *param_ptr, ub param_len, ub alarm_ms);
RetCode __base_timer_die__(TIMERID timer_id, s8 *fun, ub line);
#define base_timer_die(timer_id) __base_timer_die__(timer_id, (s8 *)__func__, (ub)__LINE__)
RetCode __base_timer_kill__(char *name, s8 *fun, ub line);
#define base_timer_kill(name) __base_timer_kill__(name, (s8 *)__func__, (ub)__LINE__)

void base_timer_init(void);
void base_timer_exit(void);

#endif


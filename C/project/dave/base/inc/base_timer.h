/*
 * ================================================================================
 * (c) Copyright 2016 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#ifndef __BASE_TIMER_H__
#define __BASE_TIMER_H__

#define INVALID_TIMER_ID (-1)

typedef sb TIMERID;
typedef void (* base_timer_fun)(TIMERID timer_id, ub thread_index);
typedef void (* base_timer_param_fun)(TIMERID timer_id, ub thread_index, void *param);

TIMERID base_timer_creat(char *name, base_timer_fun fun, ub alarm_ms);
TIMERID base_timer_param_creat(char *name, base_timer_param_fun fun, void *param, ub alarm_ms);
ErrCode __base_timer_die__(TIMERID timer_id, s8 *fun, ub line);

void base_timer_init(void);
void base_timer_exit(void);

#define base_timer_die(timer_id) __base_timer_die__(timer_id, (s8 *)__func__, (ub)__LINE__)


// $$$$$$$$$$$$$$$$$$interface compatible$$$$$$$$$$$$$$$$$$$$
#define dave_timer_fun base_timer_fun
#define dave_timer_param_fun base_timer_param_fun
#define dave_timer_creat base_timer_creat
#define dave_timer_param_creat base_timer_param_creat
#define dave_timer_die base_timer_die


#endif


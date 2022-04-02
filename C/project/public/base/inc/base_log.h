/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_LOG_H__
#define __BASE_LOG_H__

#include <stdarg.h>

void __base_debug__(const char *args, ...);
void __base_catcher__(const char *args, ...);
void __base_trace__(const char *args, ...);
void __base_log__(const char *args, ...);
void __base_abnormal__(const char *args, ...);
void __base_assert__(int assert_flag, const char *fun, int line, const char *args, ...);

dave_bool base_log_line_enable(s8 *fun, ub line, ub time, ub number);

ub base_log_load(s8 *log_buf, ub log_buf_len, TraceLevel *level);
ub base_log_history(s8 *log_buf, ub log_buf_len);

void base_log_init(void);
void base_log_exit(void);

void base_log_stack_init(void);
void base_log_stack_exit(void);

#define BASEDEBUG __base_debug__
#define BASECATCHER __base_catcher__
#define BASETRACE __base_trace__
#define BASELOG __base_log__
#define BASEABNORMAL __base_abnormal__
#define BASEASSERT(assert_flag, args...) __base_assert__(assert_flag, __func__, __LINE__, ## args)

#define TRACEFLAG DAVEDEBUG("%s:%d\n", (s8 *)__func__, (ub)__LINE__);
#define TRACETENABLE if(base_thread_trace_state() == dave_true)
#define TRACELENABLE(TIME, NUMBER) if(base_log_line_enable((s8 *)__func__, (ub)__LINE__, TIME, NUMBER) == dave_true)


// $$$$$$$$$$$$$$$$$$interface compatible$$$$$$$$$$$$$$$$$$$$
#define DAVEDEBUG BASEDEBUG
#define DAVECATCHER BASECATCHER
#define DAVETRACE BASETRACE
#define DAVELOG BASELOG
#define DAVEABNORMAL BASEABNORMAL
#define DAVEASSERT BASEASSERT

#endif


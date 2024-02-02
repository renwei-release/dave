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

dave_bool base_log_id_add(s8 *trace_id);
dave_bool base_log_id_del(s8 *trace_id);
dave_bool base_log_id_enable(s8 *trace_id);
dave_bool base_log_line_enable(s8 *fun, ub line, ub second, ub number);

ub base_log_load(s8 *log_ptr, ub log_len, TraceLevel *level);
dave_bool base_log_has_data(void);
ub base_log_history(s8 *history_ptr, ub history_len);

void base_log_init(void);
void base_log_exit(void);

void base_log_trace_enable(dave_bool write_cfg);
void base_log_trace_disable(dave_bool write_cfg);

void base_log_stack_init(void);
void base_log_stack_exit(void);

#define TRACEFLAG fprintf(stdout, "%s:%d\n", __func__, __LINE__);
#define TRACETENABLE if(base_thread_trace_state() == dave_true)
#define TRACEIENABLE(ID) if(base_log_id_enable(ID) == dave_true)
#define TRACELENABLE(SECOND, NUMBER) if(base_log_line_enable((s8 *)__func__, (ub)__LINE__, SECOND, NUMBER) == dave_true)

#define DAVEDEBUG __base_debug__
#define DAVECATCHER __base_catcher__
#define DAVETRACE __base_trace__
#define DAVELOG __base_log__
#define DAVEABNORMAL __base_abnormal__
#define DAVEASSERT(assert_flag, args...) __base_assert__(assert_flag, __func__, __LINE__, ## args)

#endif


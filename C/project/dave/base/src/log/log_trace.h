/*
 * ================================================================================
 * (c) Copyright 2017 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2017.10.02.
 * ================================================================================
 */

#include "base_macro.h"
#ifndef __LOG_TRACE_H__
#define __LOG_TRACE_H__

void log_trace_init(void);

void log_trace_exit(void);

dave_bool log_trace_add_id(s8 *trace_id);

dave_bool log_trace_del_id(s8 *trace_id);

dave_bool log_trace_id_enable(s8 *trace_id);

dave_bool log_trace_line_enable(s8 *fun, ub line, ub time, ub number);

#endif


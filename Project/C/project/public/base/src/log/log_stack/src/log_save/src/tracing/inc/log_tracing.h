/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __LOG_TRACING_H__
#define __LOG_TRACING_H__

void log_tracing_init(void);

void log_tracing_exit(void);

dave_bool log_tracing(s8 *chain_id, void *pJson);

#endif


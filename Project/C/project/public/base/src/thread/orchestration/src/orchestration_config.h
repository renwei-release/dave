/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __ORCHESTRATION_CONFIG_H__
#define __ORCHESTRATION_CONFIG_H__
#include "base_macro.h"
#include "dave_base.h"
#include "thread_struct.h"

#define DAVE_OR_UID_LEN DAVE_UID_LEN
#define DAVE_OR_ROUTER_TABLE_MAX 32
#define DAVE_OR_GID_TABLE_MAX 64

typedef struct {
	ub load_balancer;
	ub gid_number;
	s8 gid_table[DAVE_OR_GID_TABLE_MAX][DAVE_GLOBALLY_IDENTIFIER_LEN];
} ORUIDGIDTable;

typedef struct {
	s8 thread[DAVE_THREAD_NAME_LEN];
	ORUIDGIDTable *pGIDTable;
} ORUIDRouter;

typedef struct {
	s8 uid[DAVE_OR_UID_LEN];
	ub router_number;
	ORUIDRouter router_table[DAVE_OR_ROUTER_TABLE_MAX];
} ORUIDConfig;

void orchestration_config_init(void);

void orchestration_config_exit(void);

ORUIDConfig * orchestration_config(s8 *uid);

ub orchestration_config_info(s8 *info_ptr, ub info_len);

#endif


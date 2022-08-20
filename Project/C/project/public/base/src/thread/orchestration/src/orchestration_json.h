/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __ORCHESTRATION_JSON_H__
#define __ORCHESTRATION_JSON_H__
#include "dave_base.h"

ORUIDConfig * or_json_malloc_config(s8 *uid, void *pArrayConfig);

void or_json_free_config(ORUIDConfig *pConfig);

#endif


/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __TRACING_LEVEL_H__
#define __TRACING_LEVEL_H__

typedef struct {
	void *pJson;
	void *next;
} GenerationList;

typedef struct {
	GenerationList *pList;
	void *next;
} GenerationLevel;

#ifdef __cplusplus
extern "C"{
#endif

GenerationLevel * tracing_level_malloc(void *pArrayJson);

void tracing_level_free(GenerationLevel *pLevel);

#ifdef __cplusplus
} //extern "C"
#endif

#endif


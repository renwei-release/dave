/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __TRACING_LEVEL_H__
#define __TRACING_LEVEL_H__

/*
 * GenerationLevel: 
 *
 * [ req <-> call_id <-> rsp ]
 *  ^- the GenerationList has one GenerationAction
 *    [ req <-> call_id <-> rsp ] ... [ req <-> call_id <-> rsp ]
 *     ^- the GenerationList has two GenerationAction
 *       [ req <-> call_id <-> rsp ]
 *        ^- the GenerationList has one GenerationAction
 */

typedef struct {
	void *pReqJson;
	void *pRspJson;
} GenerationAction;

typedef struct {
	GenerationAction action;
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


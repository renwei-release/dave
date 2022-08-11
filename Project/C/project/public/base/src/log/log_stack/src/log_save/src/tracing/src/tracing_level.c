/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "log_stack.h"
#ifdef LOG_STACK_SERVER
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "log_save_json.h"
#include "tracing_level.h"
#include "log_log.h"

#define TRACING_MAX_GENERATION 128

typedef struct {
	void *pChainJson;
	void *next;
} GenerationJsonList;

static void *
_tracing_malloc_list_find_rsp(GenerationJsonList *pFindRsp, ub req_call_id)
{
	s8 action[32];
	ub rsp_call_id;

	while(pFindRsp != NULL)
	{
		dave_json_get_str_v2(pFindRsp->pChainJson, JSON_LOG_action, action, sizeof(action));
		if(dave_strcmp(action, JSON_LOG_action_answer) == dave_true)
		{
			if(dave_json_get_ub(pFindRsp->pChainJson, JSON_LOG_call_id, &rsp_call_id) == dave_true)
			{
				if(rsp_call_id == req_call_id)
				{
					return pFindRsp->pChainJson;
				}
			}
		}

		pFindRsp = pFindRsp->next;
	}

	return NULL;
}

static GenerationJsonList *
_tracing_malloc_generation_list(void *pArrayJson, ub generation, ub *max_generation)
{
	sb array_length = dave_json_get_array_length(pArrayJson);
	sb array_index;
	void *pChainJson;
	ub chain_generation;
	GenerationJsonList *pHead = NULL;
	GenerationJsonList *pTail = NULL;
	GenerationJsonList *pList;

	*max_generation = 0;

	for(array_index=0; array_index<array_length; array_index++)
	{
		pChainJson = dave_json_get_array_idx(pArrayJson, array_index);
		if(pChainJson != NULL)
		{
			if(dave_json_get_ub(pChainJson, JSON_LOG_generation, &chain_generation) == dave_false)
			{
				LOGLOG("can't find %s on %s",
					JSON_LOG_generation,
					dave_json_to_string(pChainJson, NULL));
			}
			else
			{
				if(*max_generation < chain_generation)
				{
					*max_generation = chain_generation;
				}

				if(chain_generation == generation)
				{
					pList = dave_malloc(sizeof(GenerationJsonList));
					pList->pChainJson = pChainJson;
					pList->next = NULL;

					if(pHead == NULL)
					{
						pHead = pTail = pList;
					}
					else
					{
						pTail->next = pList;
						pTail = pList;
					}
				}
			}
		}
	}

	return pHead;
}

static void
_tracing_free_generation_list(GenerationJsonList *pList)
{
	GenerationJsonList *pTemp;

	while(pList != NULL)
	{
		pTemp = (GenerationJsonList *)(pList->next);

		dave_free(pList);

		pList = pTemp;
	}
}

static GenerationList *
_tracing_malloc_list(void *pArrayJson, ub generation, ub *max_generation)
{
	GenerationJsonList *pJsonList, *pFindReq;
	GenerationList *pHead = NULL;
	GenerationList *pTail = NULL;
	GenerationList *pList;
	s8 action[32];
	ub call_id;

	pJsonList = _tracing_malloc_generation_list(pArrayJson, generation, max_generation);
	if(pJsonList == NULL)
	{
		return NULL;
	}

	pFindReq = pJsonList;
	while(pFindReq != NULL)
	{
		dave_json_get_str_v2(pFindReq->pChainJson, JSON_LOG_action, action, sizeof(action));
		if(dave_strcmp(action, JSON_LOG_action_request) == dave_true)
		{
			pList = dave_malloc(sizeof(GenerationList));
			pList->action.pReqJson = pFindReq->pChainJson;
			if(dave_json_get_ub(pList->action.pReqJson, JSON_LOG_call_id, &call_id) == dave_true)
			{
				pList->action.pRspJson = _tracing_malloc_list_find_rsp(pJsonList, call_id);
			}
			else
			{
				pList->action.pRspJson = NULL;
			}
			pList->next = NULL;

			if(pHead == NULL)
			{
				pHead = pTail = pList;
			}
			else
			{
				pTail->next = pList;
				pTail = pList;
			}
		}

		pFindReq = pFindReq->next;
	}

	_tracing_free_generation_list(pJsonList);

	return pHead;
}

static void
_tracing_free_list(GenerationList *pList)
{
	GenerationList *pTemp;

	while(pList != NULL)
	{
		pTemp = (GenerationList *)(pList->next);

		dave_free(pList);

		pList = pTemp;
	}
}

static GenerationLevel *
_tracing_malloc_level(void *pArrayJson)
{
	ub max_generation = TRACING_MAX_GENERATION;
	ub index_generation;
	GenerationList *pList;
	GenerationLevel *pHead = NULL;
	GenerationLevel *pTail = NULL;
	GenerationLevel *pLevel;

	for(index_generation=0; index_generation<=max_generation; index_generation++)
	{
		pList = _tracing_malloc_list(pArrayJson, index_generation, &max_generation);
		if(pList == NULL)
		{
			break;
		}

		pLevel = dave_malloc(sizeof(GenerationLevel));

		pLevel->pList = pList;
		pLevel->next = NULL;

		if(pHead == NULL)
		{
			pHead = pTail = pLevel;
		}
		else
		{
			pTail->next = pLevel;
			pTail = pLevel;
		}
	}

	return pHead;
}

static void
_tracing_free_level(GenerationLevel *pLevel)
{
	GenerationLevel *pTemp;

	while(pLevel != NULL)
	{
		pTemp = (GenerationLevel *)(pLevel->next);

		_tracing_free_list(pLevel->pList);

		dave_free(pLevel);

		pLevel = pTemp;
	}
}

// =====================================================================

GenerationLevel *
tracing_level_malloc(void *pArrayJson)
{
	return _tracing_malloc_level(pArrayJson);
}

void
tracing_level_free(GenerationLevel *pLevel)
{
	_tracing_free_level(pLevel);
}

#endif


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

static GenerationList *
_tracing_malloc_list(void *pArrayJson, ub generation, ub *max_generation)
{
	sb array_length = dave_json_get_array_length(pArrayJson);
	sb array_index;
	void *pChainJson;
	ub chain_generation;
	GenerationList *pHead = NULL;
	GenerationList *pTail = NULL;
	GenerationList *pList;

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
					pList = dave_malloc(sizeof(GenerationList));
					pList->pJson = pChainJson;
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


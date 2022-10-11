/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "log_stack.h"
#ifdef LOG_STACK_SERVER
#include <iostream>
#include <chrono>
#include "yaml-cpp/yaml.h"
#include "jaegertracing/Tracer.h"
#include "opentracing/span.h"
#include "opentracing/noop.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "thread_chain.h"
#include "log_save_json_define.h"
#include "tracing_level.h"
#include "tracing_fixed_bug.h"
#include "log_log.h"

using namespace opentracing;

static inline s8 *
_tracing_v2_span_name(s8 *name_ptr, ub name_len, GenerationAction *pAction)
{
	ub name_index = 0;

	if((pAction->pReqJson != NULL) && (pAction->pRspJson != NULL))
	{
		name_index += dave_json_get_str_v2(pAction->pReqJson, JSON_LOG_src_thread, &name_ptr[name_index], name_len-name_index);
		name_index += dave_snprintf(&name_ptr[name_index], name_len-name_index, "<->");
		name_index += dave_json_get_str_v2(pAction->pRspJson, JSON_LOG_src_thread, &name_ptr[name_index], name_len-name_index);
	}
	else if(pAction->pReqJson != NULL)
	{
		name_index += dave_json_get_str_v2(pAction->pReqJson, JSON_LOG_src_thread, &name_ptr[name_index], name_len-name_index);
		name_index += dave_snprintf(&name_ptr[name_index], name_len-name_index, "<-|");
		name_index += dave_json_get_str_v2(pAction->pReqJson, JSON_LOG_dst_thread, &name_ptr[name_index], name_len-name_index);
	}
	else if(pAction->pRspJson != NULL)
	{
		name_index += dave_json_get_str_v2(pAction->pRspJson, JSON_LOG_src_thread, &name_ptr[name_index], name_len-name_index);
		name_index += dave_snprintf(&name_ptr[name_index], name_len-name_index, "|->");
		name_index += dave_json_get_str_v2(pAction->pRspJson, JSON_LOG_dst_thread, &name_ptr[name_index], name_len-name_index);
	}
	else
	{
		name_ptr[0] = '\0';
	}

	return name_ptr;
}

static inline s8 *
_tracing_v2_tag_name(s8 *name_ptr, ub name_len, void *pJson)
{
	ub name_index = 0;

	name_index += dave_json_get_str_v2(pJson, JSON_LOG_src_thread, &name_ptr[name_index], name_len-name_index);
	name_index += dave_snprintf(&name_ptr[name_index], name_len-name_index, "->");
	name_index += dave_json_get_str_v2(pJson, JSON_LOG_dst_thread, &name_ptr[name_index], name_len-name_index);
	name_index += dave_snprintf(&name_ptr[name_index], name_len-name_index, ":");
	name_index += dave_json_get_str_v2(pJson, JSON_LOG_msg_id, &name_ptr[name_index], name_len-name_index);

	return name_ptr;
}

static inline s8 *
_tracing_v2_tag_value(void *pJson)
{
	return dave_json_to_string(pJson, NULL);
}

static inline void
_tracing_save_level_v2(GenerationLevel *pLevel)
{
	s8 span_name[128];
	s8 tag_name[128];
	const opentracing::SpanContext *parent_context;
	GenerationList *pList;

	if(pLevel->pList->next != NULL)
	{
		LOGLOG("There can only be one start call! %s",
			dave_json_to_string(pLevel->pList->action.pReqJson, NULL));
		return;
	}

	_tracing_v2_span_name(span_name, sizeof(span_name), &(pLevel->pList->action));
	auto parent_span = opentracing::Tracer::Global()->StartSpan(span_name);
	parent_span->SetTag(_tracing_v2_tag_name(tag_name, sizeof(tag_name), pLevel->pList->action.pReqJson), _tracing_v2_tag_value(pLevel->pList->action.pReqJson));
	parent_span->SetTag(_tracing_v2_tag_name(tag_name, sizeof(tag_name), pLevel->pList->action.pRspJson), _tracing_v2_tag_value(pLevel->pList->action.pRspJson));

	pLevel = (GenerationLevel *)(pLevel->next);
	parent_context = &(parent_span->context());

	while(pLevel != NULL)
	{
		pList = pLevel->pList;

		while(pList != NULL)
		{
			_tracing_v2_span_name(span_name, sizeof(span_name), &(pList->action));
			auto child_span = opentracing::Tracer::Global()->StartSpan(span_name, { opentracing::ChildOf(parent_context) });			
			if(pList->action.pReqJson != NULL)
			{
				child_span->SetTag(_tracing_v2_tag_name(tag_name, sizeof(tag_name), pList->action.pReqJson), _tracing_v2_tag_value(pList->action.pReqJson));
			}
			if(pList->action.pRspJson != NULL)
			{
				child_span->SetTag(_tracing_v2_tag_name(tag_name, sizeof(tag_name), pList->action.pRspJson), _tracing_v2_tag_value(pList->action.pRspJson));
			}
		
			pList = (GenerationList *)(pList->next);
		
			if(pList == NULL)
			{
				parent_context = &(child_span->context());
			}
		}

		pLevel = (GenerationLevel *)(pLevel->next);
	}

	parent_span->Finish();
}

// =====================================================================

void
tracing_save_level_v2(GenerationLevel *pLevel)
{
	if((pLevel == NULL) || (pLevel->pList == NULL))
	{
		return;
	}

	_tracing_save_level_v2(pLevel);
}

#endif


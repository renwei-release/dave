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

static s8 *
_tracing_span_name(s8 *name_ptr, ub name_len, void *pJson)
{
	ub name_index = 0;

	name_index += dave_json_get_str_v2(pJson, JSON_LOG_src_thread, &name_ptr[name_index], name_len-name_index);
	name_index += dave_snprintf(&name_ptr[name_index], name_len-name_index, "->");
	name_index += dave_json_get_str_v2(pJson, JSON_LOG_dst_thread, &name_ptr[name_index], name_len-name_index);
	name_index += dave_snprintf(&name_ptr[name_index], name_len-name_index, ":");
	dave_json_get_str_v2(pJson, JSON_LOG_msg_id, &name_ptr[name_index], name_len-name_index);

	return name_ptr;
}

static s8 *
_tracing_tag_name(s8 *name_ptr, ub name_len, void *pJson)
{
	ub name_index = 0;

	name_index += dave_json_get_str_v2(pJson, JSON_LOG_src_thread, &name_ptr[name_index], name_len-name_index);
	name_index += dave_snprintf(&name_ptr[name_index], name_len-name_index, "->");
	dave_json_get_str_v2(pJson, JSON_LOG_dst_thread, &name_ptr[name_index], name_len-name_index);

	return name_ptr;
}

static s8 *
_tracing_tag_value(void *pJson)
{
	return dave_json_to_string(pJson, NULL);
}

static s8 *
_tracing_span_param(s8 *span_ptr, ub span_len, s8 *tag_ptr, ub tag_len, void *pJson)
{
	if(pJson == NULL)
		return NULL;

	_tracing_span_name(span_ptr, span_len, pJson);
	_tracing_tag_name(tag_ptr, tag_len, pJson);
	return _tracing_tag_value(pJson);
}

static const opentracing::SpanContext *
_tracing_span_rsp(const opentracing::SpanContext *parent_context, void *pJson)
{
	s8 span_name[128];
	s8 tag_name[128];
	s8 *tag_value;

	if((parent_context == NULL) || (pJson == NULL))
	{
		return parent_context;
	}

	tag_value = _tracing_span_param(
		span_name, sizeof(span_name),
		tag_name, sizeof(tag_name),
		pJson);

	auto rsp_span = opentracing::Tracer::Global()->StartSpan(span_name, { opentracing::ChildOf(parent_context) });

	rsp_span->SetTag(tag_name, tag_value);

	rsp_span->Finish();

	return &rsp_span->context();
}

static const opentracing::SpanContext *
_tracing_save_level_recursion(const opentracing::SpanContext *parent_context, GenerationLevel *pLevel)
{
	s8 span_name[128];
	s8 tag_name[128];
	s8 *tag_value;
	GenerationList *pList;
	const opentracing::SpanContext *return_context = NULL;

	if((parent_context == NULL) || (pLevel == NULL))
	{
		return parent_context;
	}

	pList = pLevel->pList;
	while(pList != NULL)
	{
		tag_value = _tracing_span_param(
			span_name, sizeof(span_name),
			tag_name, sizeof(tag_name),
			pList->action.pReqJson);
		auto req_span = opentracing::Tracer::Global()->StartSpan(span_name, { opentracing::ChildOf(parent_context) });
		req_span->SetTag(tag_name, tag_value);

		return_context = _tracing_save_level_recursion(&req_span->context(), (GenerationLevel *)(pLevel->next));

		return_context = _tracing_span_rsp(return_context, pList->action.pRspJson);

		req_span->Finish();

		pList = (GenerationList *)(pList->next);
	}

	return return_context;
}

static void
_tracing_save_level(GenerationLevel *pLevel)
{
	s8 span_name[128];
	s8 tag_name[128];
	s8 *tag_value;
	const opentracing::SpanContext *return_context;

	tag_value = _tracing_span_param(
		span_name, sizeof(span_name),
		tag_name, sizeof(tag_name),
		pLevel->pList->action.pReqJson);
	auto req_span = opentracing::Tracer::Global()->StartSpan(span_name);
	req_span->SetTag(tag_name, tag_value);

	return_context = _tracing_save_level_recursion(&req_span->context(), (GenerationLevel *)(pLevel->next));

	if(return_context != NULL)
	{
		_tracing_span_rsp(return_context, pLevel->pList->action.pRspJson);
	}

	req_span->Finish();
}

// =====================================================================

void
tracing_save_level_v1(GenerationLevel *pLevel)
{
	if((pLevel == NULL)
		|| (pLevel->pList == NULL)
		|| (pLevel->pList->action.pReqJson == NULL))
	{
		return;
	}

	_tracing_save_level(pLevel);
}

#endif


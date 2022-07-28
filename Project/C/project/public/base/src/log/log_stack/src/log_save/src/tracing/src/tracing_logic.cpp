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
#include "yaml-cpp/yaml.h"
#include "jaegertracing/Tracer.h"
#include "opentracing/span.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_3rdparty.h"
#include "thread_chain.h"
#include "log_save_json.h"
#include "tracing_level.h"
#include "tracing_fixed_bug.h"
#include "log_log.h"

#define TRACING_CONFIG_FILE "/dave/tools/jaeger-client-cpp/config/config.yml"
#define TRACING_SERVER_NAME "DAVE-CHAIN"

static jaegertracing::Config _tracing_config;

static void
_tracing_config_build(const char* configFilePath)
{
    auto configYAML = YAML::LoadFile(configFilePath);

    _tracing_config = jaegertracing::Config::parse(configYAML);
}

static void
_tracing_global_init(void)
{
    auto tracer = jaegertracing::Tracer::make(TRACING_SERVER_NAME, _tracing_config, jaegertracing::logging::consoleLogger());

    opentracing::Tracer::InitGlobal(std::static_pointer_cast<opentracing::Tracer>(tracer));
}

static void
_tracing_global_exit(void)
{
	opentracing::Tracer::Global()->Close();
}

static s8 *
_tracing_span_name(s8 *name_ptr, ub name_len, GenerationList *pList)
{
	ub name_index;
	s8 action[16];
	s8 src_thread[DAVE_CHAIN_THREAD_NAME_LEN];
	s8 dst_thread[DAVE_CHAIN_THREAD_NAME_LEN];
	s8 msg_id[128];
	GenerationList *pWhileList, *pAnswerList = NULL;

	name_ptr[0] = '\0';
	name_index = 0;

	pWhileList = pList;
	while(pWhileList != NULL)
	{
		if(dave_json_get_str_v2(pWhileList->pJson, JSON_LOG_action, action, sizeof(action)) > 0)
		{
			if(dave_strcmp(action, JSON_LOG_action_request) == dave_true)
			{
				dave_json_get_str_v2(pWhileList->pJson, JSON_LOG_src_thread, src_thread, sizeof(src_thread));
				dave_json_get_str_v2(pWhileList->pJson, JSON_LOG_dst_thread, dst_thread, sizeof(dst_thread));
				dave_json_get_str_v2(pWhileList->pJson, JSON_LOG_msg_id, msg_id, sizeof(msg_id));
				name_index += dave_snprintf(&name_ptr[name_index], name_len-name_index, "%s(%s)->%s", src_thread, msg_id, dst_thread);
				break;
			}
			else
			{
				pAnswerList = pWhileList;
			}
		}
		pWhileList = (GenerationList *)(pWhileList->next);
	}

	if(pAnswerList != NULL)
		pWhileList = pAnswerList;
	else
		pWhileList = pList;
	while(pWhileList != NULL)
	{
		if(dave_json_get_str_v2(pWhileList->pJson, JSON_LOG_action, action, sizeof(action)) > 0)
		{
			if(dave_strcmp(action, JSON_LOG_action_answer) == dave_true)
			{
				dave_json_get_str_v2(pWhileList->pJson, JSON_LOG_dst_thread, dst_thread, sizeof(dst_thread));
				dave_json_get_str_v2(pWhileList->pJson, JSON_LOG_msg_id, msg_id, sizeof(msg_id));
				name_index += dave_snprintf(&name_ptr[name_index], name_len-name_index, "(%s)->%s", msg_id, dst_thread);
				break;
			}
		}
		pWhileList = (GenerationList *)(pWhileList->next);
	}

	if(name_index == 0)
	{
		LOGLOG("can't get any name!");
	}

	return name_ptr;
}

static s8 *
_tracing_tag_name(s8 *name_ptr, ub name_len, GenerationList *pList)
{
	s8 src_thread[DAVE_CHAIN_THREAD_NAME_LEN];
	s8 dst_thread[DAVE_CHAIN_THREAD_NAME_LEN];

	dave_json_get_str_v2(pList->pJson, JSON_LOG_src_thread, src_thread, sizeof(src_thread));
	dave_json_get_str_v2(pList->pJson, JSON_LOG_dst_thread, dst_thread, sizeof(dst_thread));
	dave_snprintf(name_ptr, name_len, "%s->%s", src_thread, dst_thread);

	return name_ptr;
}

static void
_tracing_save_level(GenerationLevel *pLevel)
{
	GenerationList *pParentList, *pChildList;
	const opentracing::SpanContext *parent_span_context;
	s8 span_name[512];
	s8 tag_name[128];
	s8 *tag_value;

	if(pLevel == NULL)
	{
		return;
	}

	pParentList = pLevel->pList;
	if(pParentList == NULL)
	{
		return;
	}

	_tracing_span_name(span_name, sizeof(span_name), pParentList);
	auto parent_span = opentracing::Tracer::Global()->StartSpan(span_name);

	while(pParentList != NULL)
	{
		_tracing_tag_name(tag_name, sizeof(tag_name), pParentList);
		tag_value = dave_json_to_string(pParentList->pJson, NULL);

		parent_span->SetTag(tag_name, tag_value);

		pParentList = (GenerationList *)(pParentList->next);
	}

	parent_span_context = &parent_span->context();
	pLevel = (GenerationLevel *)(pLevel->next);

	while(pLevel != NULL)
	{
		pChildList = (GenerationList *)(pLevel->pList);
	
		_tracing_span_name(span_name, sizeof(span_name), pChildList);
		auto child_span = opentracing::Tracer::Global()->StartSpan(span_name, { opentracing::ChildOf(parent_span_context) } );

		while(pChildList != NULL)
		{
			_tracing_tag_name(tag_name, sizeof(tag_name), pChildList);
			tag_value = dave_json_to_string(pChildList->pJson, NULL);

			child_span->SetTag(tag_name, tag_value);
		
			pChildList = (GenerationList *)(pChildList->next);
		}
		child_span->Finish();

		parent_span_context = &child_span->context();
		pLevel = (GenerationLevel *)(pLevel->next);
	}

	parent_span->Finish();
}

// =====================================================================

extern "C" void
tracing_logic_init(void)
{
	_tracing_config_build(TRACING_CONFIG_FILE);

	_tracing_global_init();
}

extern "C" void
tracing_logic_exit(void)
{
	_tracing_global_exit();
}

extern "C" void
tracing_logic(void *pArrayJson)
{
	GenerationLevel *pLevel;

	LOGDEBUG("%s", dave_json_to_string(pArrayJson, NULL));

	pLevel = tracing_level_malloc(pArrayJson);

	tracing_fixed_bug(pArrayJson);

	_tracing_save_level(pLevel);

	tracing_level_free(pLevel);
}

#endif


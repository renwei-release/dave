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
#include "log_save_json.h"
#include "tracing_level.h"
#include "tracing_fixed_bug.h"
#include "tracing_logic_v1.h"
#include "tracing_logic_v2.h"
#include "log_log.h"

using namespace opentracing;

typedef enum {
	TracingSaveLevelType_v1,
	TracingSaveLevelType_v2,
} TracingSaveLevelType;

#define TRACING_CONFIG_FILE "/dave/tools/jaeger-client-cpp/config/config.yml"
#define TRACING_SERVER_NAME "DAVE-CHAIN"
#define CFG_TRACE_SAVE_LEVEL_TYPE (s8 *)"TraceSaveLevelType"

static jaegertracing::Config _tracing_config;
static TracingSaveLevelType _save_level_type = TracingSaveLevelType_v2;

static void
_tracing_save_level_type(void)
{
	s8 type_str[128];

	cfg_get_by_default(CFG_TRACE_SAVE_LEVEL_TYPE, type_str, sizeof(type_str), (s8 *)"v2");

	if(dave_strcmp(type_str, (s8 *)"v1") == dave_true)
	{
		_save_level_type = TracingSaveLevelType_v1;
	}
	else if(dave_strcmp(type_str, (s8 *)"v2") == dave_true)
	{
		_save_level_type = TracingSaveLevelType_v2;
	}
	else
	{
		_save_level_type = TracingSaveLevelType_v2;
	}
}

static void
_tracing_config_build(const char* configFilePath)
{
    auto configYAML = YAML::LoadFile(configFilePath);

    _tracing_config = jaegertracing::Config::parse(configYAML);
}

static void
_tracing_global_init(void)
{
    auto tracer = jaegertracing::Tracer::make(TRACING_SERVER_NAME, _tracing_config, jaegertracing::logging::nullLogger());

    opentracing::Tracer::InitGlobal(std::static_pointer_cast<opentracing::Tracer>(tracer));
}

static void
_tracing_global_exit(void)
{
	opentracing::Tracer::Global()->Close();
}

static void
_tracing_level_save(GenerationLevel *pLevel)
{
	if(_save_level_type == TracingSaveLevelType_v1)
	{
		tracing_save_level_v1(pLevel);
	}
	else
	{
		tracing_save_level_v2(pLevel);
	}
}

// =====================================================================

extern "C" void
tracing_logic_init(void)
{
	_tracing_save_level_type();

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

	_tracing_level_save(pLevel);

	tracing_level_free(pLevel);
}

#endif


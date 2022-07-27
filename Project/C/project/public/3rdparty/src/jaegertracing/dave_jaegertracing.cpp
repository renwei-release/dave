/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(JAEGERTRACING_3RDPARTY)
#include <iostream>
#include "yaml-cpp/yaml.h"
#include "jaegertracing/Tracer.h"
#include "dave_os.h"
#include "party_log.h"

#define JT_CONFIG_FILE "/dave/tools/jaeger-client-cpp/config/config.yml"

static jaegertracing::Config _jaegertracing_config;

static void
_jaegertracing_config_build(const char* configFilePath)
{
    auto configYAML = YAML::LoadFile(configFilePath);

    _jaegertracing_config = jaegertracing::Config::parse(configYAML);
}

// =====================================================================

extern "C" void
dave_jaegertracing_init(void)
{
	_jaegertracing_config_build(JT_CONFIG_FILE);

	auto tracer = jaegertracing::Tracer::make("CHAIN", _jaegertracing_config, jaegertracing::logging::consoleLogger());

	auto span = tracer->StartSpan("IO");

	auto span_a = tracer->StartSpan("A", { opentracing::ChildOf(&span->context()) });
	dave_os_sleep(100);
	span_a->Finish();

	auto span_b = tracer->StartSpan("B", { opentracing::ChildOf(&span_a->context()) });
	dave_os_sleep(100);
	span_b->Finish();

	auto span_c = tracer->StartSpan("C", { opentracing::ChildOf(&span_b->context()) });
	dave_os_sleep(100);
	span_c->Finish();

	span->Finish();

	tracer->Close();
}

extern "C" void
dave_jaegertracing_exit(void)
{

}

#endif


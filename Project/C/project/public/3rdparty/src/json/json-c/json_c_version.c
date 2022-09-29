/*
 * Copyright (c) 2012 Eric Haszlakiewicz
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 */
#include "3rdparty_macro.h"
#if defined(JSON_3RDPARTY)
#include "dave_base.h"

#include "config.h"

#include "json_c_version.h"

const char *json_c_version(void)
{
	return JSON_C_VERSION;
}

int json_c_version_num(void)
{
	return JSON_C_VERSION_NUM;
}

#endif


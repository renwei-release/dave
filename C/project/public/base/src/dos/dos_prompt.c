/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "dos_cmd.h"
#include "dos_welcome.h"
#include "dos_log.h"

static s8 _dos_prompt[256];

// =====================================================================

s8 *
dos_prompt(void)
{
	dave_sprintf(_dos_prompt, "%s:> ", dave_verno_my_product());

	return _dos_prompt;
}

#endif


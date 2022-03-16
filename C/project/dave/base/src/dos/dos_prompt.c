/*
 * ================================================================================
 * (c) Copyright 2016 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
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
	dave_sprintf(_dos_prompt, "%s:> ", dave_verno_product(NULL, NULL, 0));

	return _dos_prompt;
}

#endif


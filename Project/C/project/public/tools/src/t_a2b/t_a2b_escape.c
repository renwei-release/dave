/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "tools_log.h"

// =====================================================================

ub
t_a2b_string_to_escape(s8 *escape_ptr, ub escape_len, s8 *string)
{
	ub escape_index;

	if(string == NULL)
	{
		escape_ptr[0] = '\0';
		return 0;
	}

	escape_index = 0;

	while(((escape_index + 2) < escape_len) && (*string != '\0'))
	{
		if(*string == '"')
			escape_ptr[escape_index ++] = '\\';
		escape_ptr[escape_index ++] = *string;

		string ++;
	}

	if(escape_index < escape_len)
	{
		escape_ptr[escape_index] = '\0';
	}

	return escape_index;
}


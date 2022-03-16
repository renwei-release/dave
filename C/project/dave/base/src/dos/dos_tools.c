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
#include "dos_tools.h"
#include "dos_show.h"
#include "dos_log.h"

// =====================================================================

ub
dos_get_last_parameters(s8 *cmd_ptr, ub cmd_len, s8 *param_ptr, ub param_len)
{
	ub cmd_index;

	if((cmd_ptr == NULL) || (cmd_len == 0) 
		|| (param_ptr == NULL) || (param_len == 0))
	{
		if((param_ptr != NULL) && (param_len > 0))
		{
			param_ptr[0] = '\0';
		}
		return 0;
	}

	cmd_index = 0;

	while((cmd_index < cmd_len) 
		&& ((t_is_show_char(cmd_ptr[cmd_index]) == dave_false) || (cmd_ptr[cmd_index] == ' ')))
	{
		cmd_index ++;
	}

	if(param_len > (cmd_len - cmd_index))
	{
		param_len = (cmd_len - cmd_index) + 1;
	}

	return dave_strcpy(param_ptr, &cmd_ptr[cmd_index], param_len);
}

ub
dos_get_one_parameters(s8 *cmd_ptr, ub cmd_len, s8 *param_ptr, ub param_len)
{
	ub cmd_index, param_index;

	if((cmd_ptr == NULL) || (cmd_len == 0) 
		|| (param_ptr == NULL) || (param_len == 0))
	{
		if((param_ptr != NULL) && (param_len > 0))
		{
			param_ptr[0] = '\0';
		}
		return 0;
	}

	cmd_index = 0;

	while((cmd_index < cmd_len) 
		&& ((t_is_show_char(cmd_ptr[cmd_index]) == dave_false) || (cmd_ptr[cmd_index] == ' ')))
	{
		cmd_index ++;
	}

	param_index = 0;

	while((cmd_index < cmd_len) 
		&& (cmd_ptr[cmd_index] != ' ')
		&& ((param_index + 1) < param_len))
	{
		param_ptr[param_index ++] = cmd_ptr[cmd_index ++];
	}

	if((param_index + 1) >  param_len)
	{
		param_ptr[0] = '\0';
	}
	else
	{
		param_ptr[param_index] = '\0';
	}

	return cmd_index;
}

#endif

